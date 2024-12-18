/*
  TEROS-12 Sensor Monitor with Home Assistant Integration
  For ESP32 with SDI-12 sensor
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <esp32-sdi12.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <time.h>

// WiFi and MQTT Settings
const char* WIFI_SSID = "Wifi SSID";
const char* WIFI_PASSWORD = "Wifi PW";
const char* MQTT_SERVER = "192.168.1.XX;
const int MQTT_PORT = 1883;
const char* MQTT_USER = "mqtt";
const char* MQTT_PASSWORD = "ttqm";
const char* DEVICE_NAME = "Zone 1-1";
const char* MQTT_TOPIC_BASE = "homeassistant/sensor/sdi12";

// Device Information
const char* MANUFACTURER = "China No.1";
const char* MODEL = "Teros";
const char* SW_VERSION = "1.0.0";
const uint8_t SDI12_ADDRESS = 0;
const int SDI12_DATA_PIN = 26;

// Constants
const float ROCKWOOL_TOTAL_POROSITY = 0.95;
const float OFFSET_PERMITTIVITY = 4.1;
const int MAX_LOG_ENTRIES = 20;
const unsigned long READING_INTERVAL = 15000;  // 15 seconds
const unsigned long AVAILABILITY_INTERVAL = 60000;  // 1 minute

// MQTT Topics
String stateTopic;
String availabilityTopic;
String commandTopic;
String errorTopic;

// Global objects
ESP32_SDI12 sdi12(SDI12_DATA_PIN);
WiFiClient espClient;
PubSubClient mqttClient(espClient);
WebServer server(80);

// Sensor readings structure
struct SensorReadings {
    float raw_vwc;
    float vwc;
    float temperature;
    float bulk_ec;
    float temp_compensated_ec;
    float pore_water_ec;
    float saturation_extract_ec;
} readings;

// Global variables
float values[10];  // Buffer for SDI-12 readings
unsigned long lastReconnectAttempt = 0;
unsigned long mqttReconnectInterval = 5000;
String dataLog;
char mqtt_client_id[50];

// Calculation functions
float calculateVWC(float raw) {
    return (6.771e-10 * pow(raw, 3) - 5.105e-6 * pow(raw, 2) + 1.302e-2 * raw - 10.848);
}

float calculateBulkPermittivity(float raw) {
    return pow(2.887e-9 * pow(raw, 3) - 2.080e-5 * pow(raw, 2) + 5.276e-2 * raw - 43.39, 2);
}

float calculatePoreWaterEC(float bulkEC, float soilTemp, float bulkPermittivity) {
    float waterPermittivity = 80.3 - 0.37 * (soilTemp - 20);
    return (waterPermittivity * bulkEC) / (bulkPermittivity - OFFSET_PERMITTIVITY);
}

float compensateECForTemperature(float ec, float temperature) {
    return ec / (1 + 0.02 * (temperature - 25));
}

float calculateSaturationExtractEC(float poreWaterEC, float vwc) {
    return (poreWaterEC * vwc) / ROCKWOOL_TOTAL_POROSITY;
}

String getTimestamp() {
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)) {
        return "Time not set";
    }
    char timeStringBuff[50];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return String(timeStringBuff);
}

void addToDataLog(const String& entry) {
    dataLog = entry + "\n" + dataLog;
    int newlineCount = 0;
    for (int i = 0; i < dataLog.length(); i++) {
        if (dataLog[i] == '\n') {
            newlineCount++;
            if (newlineCount > MAX_LOG_ENTRIES) {
                dataLog = dataLog.substring(0, i);
                break;
            }
        }
    }
}

void handleSensorReadings() {
    Serial.println("\nTaking sensor readings...");
    
    // First check if sensor is responding
    if (sdi12.ackActive(SDI12_ADDRESS) != ESP32_SDI12::SDI12_OK) {
        Serial.println("Error: Sensor not responding");
        mqttClient.publish(errorTopic.c_str(), "Sensor not responding", true);
        return;
    }

    // Take measurement using the library's measure function
    ESP32_SDI12::Status res = sdi12.measure(SDI12_ADDRESS, values, 10);
    
    if (res != ESP32_SDI12::SDI12_OK) {
        Serial.printf("Error taking measurement: %d\n", res);
        mqttClient.publish(errorTopic.c_str(), "Measurement failed", true);
        return;
    }

    // Process readings (TEROS-12 returns VWC, temp, and EC in that order)
    readings.raw_vwc = values[0];
    readings.temperature = values[1];
    readings.bulk_ec = values[2] / 1000.0;  // Convert µS/cm to dS/m
    
    Serial.println("Raw readings:");
    Serial.printf("Raw VWC: %.2f\n", readings.raw_vwc);
    Serial.printf("Temperature: %.2f°C\n", readings.temperature);
    Serial.printf("Bulk EC: %.3f dS/m\n", readings.bulk_ec);
    
    if (!isnan(readings.raw_vwc) && !isnan(readings.temperature) && !isnan(readings.bulk_ec)) {
        // Calculate derived values
        readings.vwc = calculateVWC(readings.raw_vwc);
        float bulk_permittivity = calculateBulkPermittivity(readings.raw_vwc);
        readings.temp_compensated_ec = compensateECForTemperature(readings.bulk_ec, readings.temperature);
        
        if (readings.vwc >= 0.10) {
            readings.pore_water_ec = calculatePoreWaterEC(readings.temp_compensated_ec, 
                                                        readings.temperature, 
                                                        bulk_permittivity);
            readings.saturation_extract_ec = calculateSaturationExtractEC(readings.pore_water_ec, 
                                                                        readings.vwc);
        } else {
            readings.pore_water_ec = 0;
            readings.saturation_extract_ec = 0;
            Serial.println("VWC too low for accurate EC calculations");
        }
        
        // Publish to MQTT
        DynamicJsonDocument doc(512);
        char buffer[512];
        
        doc["raw_vwc"] = readings.raw_vwc;
        doc["vwc"] = readings.vwc * 100;  // Convert to percentage
        doc["temperature"] = readings.temperature;
        doc["bulk_ec"] = readings.bulk_ec;
        doc["temp_comp_ec"] = readings.temp_compensated_ec;
        doc["pore_water_ec"] = readings.pore_water_ec;
        doc["saturation_extract_ec"] = readings.saturation_extract_ec;
        
        size_t n = serializeJson(doc, buffer);
        
        Serial.println("Publishing to MQTT:");
        Serial.println("Topic: " + stateTopic);
        Serial.println("Payload: " + String(buffer));
        
        if(mqttClient.publish(stateTopic.c_str(), buffer, true)) {
            Serial.println("MQTT publish successful");
        } else {
            Serial.println("MQTT publish failed");
        }
    } else {
        Serial.println("Invalid readings detected");
        mqttClient.publish(errorTopic.c_str(), "Invalid sensor readings", true);
    }
}

void publishHomeAssistantConfig() {
    Serial.println("\nPublishing Home Assistant MQTT Discovery config...");
    
    struct SensorConfig {
        const char* type;
        const char* name;
        const char* unit;
        const char* device_class;
        const char* value_template;
    };
    
    SensorConfig sensors[] = {
        {"vwc", "VWC", "%", "humidity", "{{ value_json.vwc|round(2) }}"},
        {"temperature", "Temperature", "°C", "temperature", "{{ value_json.temperature|round(2) }}"},
        {"bulk_ec", "Bulk EC", "dS/m", "electrical_conductivity", "{{ value_json.bulk_ec|round(3) }}"},
        {"temp_comp_ec", "Temp Comp EC", "dS/m", "electrical_conductivity", "{{ value_json.temp_comp_ec|round(3) }}"},
        {"pore_water_ec", "Pore Water EC", "dS/m", "electrical_conductivity", "{{ value_json.pore_water_ec|round(3) }}"},
        {"sat_extract_ec", "Saturation Extract EC", "dS/m", "electrical_conductivity", "{{ value_json.saturation_extract_ec|round(3) }}"},
        {"raw_vwc", "Raw VWC", "", nullptr, "{{ value_json.raw_vwc|round(2) }}"}
    };
    
    for (const auto& sensor : sensors) {
        String discoveryTopic = String(MQTT_TOPIC_BASE) + "/" + DEVICE_NAME + "/" + sensor.type + "/config";
        
        DynamicJsonDocument doc(1024);
        char buffer[512];
        
        doc["name"] = String(DEVICE_NAME) + " " + sensor.name;
        doc["unique_id"] = String(DEVICE_NAME) + "_" + sensor.type;
        doc["state_topic"] = stateTopic;
        doc["availability_topic"] = availabilityTopic;
        doc["value_template"] = sensor.value_template;
        
        if (sensor.unit) {
            doc["unit_of_measurement"] = sensor.unit;
        }
        if (sensor.device_class) {
            doc["device_class"] = sensor.device_class;
        }
        
        doc["state_class"] = "measurement";
        
        JsonObject device = doc.createNestedObject("device");
        device["identifiers"][0] = DEVICE_NAME;
        device["manufacturer"] = MANUFACTURER;
        device["model"] = MODEL;
        device["name"] = DEVICE_NAME;
        device["sw_version"] = SW_VERSION;
        
        size_t n = serializeJson(doc, buffer);
        
        Serial.println("Publishing config for: " + String(sensor.type));
        Serial.println("Topic: " + discoveryTopic);
        
        if(mqttClient.publish(discoveryTopic.c_str(), buffer, true)) {
            Serial.println("Success");
        } else {
            Serial.println("Failed");
        }
        
        delay(100);
    }
}

void reconnectMQTT() {
    if (!mqttClient.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (mqttClient.connect(mqtt_client_id, MQTT_USER, MQTT_PASSWORD, 
                             availabilityTopic.c_str(), 1, true, "offline")) {
            Serial.println("connected");
            mqttClient.publish(availabilityTopic.c_str(), "online", true);
            publishHomeAssistantConfig();
            mqttReconnectInterval = 5000;
        } else {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" retry later");
            mqttReconnectInterval = min(mqttReconnectInterval * 2, 300000UL);
        }
    }
}

void setupWiFi() {
    Serial.print("Connecting to WiFi");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("\nWiFi connected");
    Serial.println("IP address: " + WiFi.localIP().toString());
}

void setupMQTT() {
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    snprintf(mqtt_client_id, sizeof(mqtt_client_id), "ESP32-%s", DEVICE_NAME);
    
    stateTopic = String(MQTT_TOPIC_BASE) + "/" + DEVICE_NAME + "/state";
    availabilityTopic = String(MQTT_TOPIC_BASE) + "/" + DEVICE_NAME + "/availability";
    commandTopic = String(MQTT_TOPIC_BASE) + "/" + DEVICE_NAME + "/command";
    errorTopic = String(MQTT_TOPIC_BASE) + "/" + DEVICE_NAME + "/error";
    
    Serial.println("MQTT Topics:");
    Serial.println("State: " + stateTopic);
    Serial.println("Availability: " + availabilityTopic);
    Serial.println("Command: " + commandTopic);
    Serial.println("Error: " + errorTopic);
}

void setup() {
    Serial.begin(115200);
    Serial.println("\nTEROS-12 Sensor Monitor Starting...");

    sdi12.begin();
    setupWiFi();
    setupMQTT();
    
    // Initialize time
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    
    Serial.println("Setup completed");
}

void loop() {
    static unsigned long lastReadingTime = 0;
    static unsigned long lastAvailabilityUpdate = 0;
    unsigned long currentMillis = millis();
    
    if (!mqttClient.connected()) {
        if (currentMillis - lastReconnectAttempt >= mqttReconnectInterval) {
            lastReconnectAttempt = currentMillis;
            reconnectMQTT();
        }
    } else {
        mqttClient.loop();
        
        // Update availability
        if (currentMillis - lastAvailabilityUpdate >= AVAILABILITY_INTERVAL) {
            lastAvailabilityUpdate = currentMillis;
            mqttClient.publish(availabilityTopic.c_str(), "online", true);
        }
        
        // Take readings
        if (currentMillis - lastReadingTime >= READING_INTERVAL) {
            lastReadingTime = currentMillis;
            handleSensorReadings();
        }
    }
}
