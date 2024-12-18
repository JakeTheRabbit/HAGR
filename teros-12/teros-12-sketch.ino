#include <WiFi.h>
#include <PubSubClient.h>
#include <esp32-sdi12.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <time.h>

// Configuration
const char* ssid = "";                      
const char* password = "";                      
const char* mqtt_server = "192.168.73.250";             
const int mqtt_port = 1883;                             
const char* mqtt_user = "";                         
const char* mqtt_password = "";                     
const char* device_name = "F1-Zone-1";              
const char* MQTT_TOPIC_BASE = "homeassistant/sensor/sdi12";
const uint8_t DEVICE_ADDRESS = 0;                       
#define SDI12_DATA_PIN 26

const char* MANUFACTURER = "METER Group";
const char* MODEL = "TEROS-12";
const char* SW_VERSION = "1.0.0";
const float ROCKWOOL_TOTAL_POROSITY = 0.95;             
const float OFFSET_PERMITTIVITY = 4.1;                  
#define VALUES_BUFFER_SIZE 10                           
const int MAX_LOG_ENTRIES = 20;
const unsigned long mqttReconnectInterval = 5000;       
const unsigned long READING_INTERVAL = 15000;           
const unsigned long AVAILABILITY_INTERVAL = 60000;      

String stateTopic;
String availabilityTopic;
String commandTopic;
String errorTopic;

char mqtt_client_id[50];
float values[VALUES_BUFFER_SIZE];
String dataLog = "";
unsigned long lastReconnectAttempt = 0;

ESP32_SDI12 sdi12(SDI12_DATA_PIN);
WiFiClient espClient;
PubSubClient client(espClient);
WebServer server(80);

struct SensorReadings {
    float raw_vwc;
    float vwc;
    float temperature;
    float bulk_ec;
    float temp_compensated_ec;
    float pore_water_ec;
    float saturation_extract_ec;
} readings;

float calculateVWC(float raw) {
    Serial.printf("Calculating VWC from raw value: %.2f\n", raw);
    float vwc = (0.0003879 * raw - 0.6956);
    vwc = vwc * vwc;
    if (vwc < 0) vwc = 0;
    if (vwc > 1) vwc = 1;
    Serial.printf("Calculated VWC: %.2f%%\n", vwc * 100);
    return vwc;
}

float calculateBulkPermittivity(float raw) {
    Serial.printf("Calculating permittivity from raw: %.2f\n", raw);
    float permittivity = pow(0.0003879 * raw + 0.6956, 2);
    Serial.printf("Calculated permittivity: %.2f\n", permittivity);
    return permittivity;
}

float calculatePoreWaterEC(float bulkEC, float soilTemp, float bulkPermittivity) {
    Serial.printf("Calculating pore water EC (bulkEC: %.3f, temp: %.2f)\n", bulkEC, soilTemp);
    float waterPermittivity = 80.3 - 0.37 * (soilTemp - 20);
    float poreWaterEC = (waterPermittivity * bulkEC) / (bulkPermittivity - OFFSET_PERMITTIVITY);
    Serial.printf("Calculated pore water EC: %.3f\n", poreWaterEC);
    return poreWaterEC;
}

float compensateECForTemperature(float ec, float temperature) {
    Serial.printf("Temperature compensating EC: %.3f at %.2f°C\n", ec, temperature);
    return ec / (1 + 0.02 * (temperature - 25));
}

float calculateSaturationExtractEC(float poreWaterEC, float vwc) {
    return (poreWaterEC * vwc) / ROCKWOOL_TOTAL_POROSITY;
}

void handleRoot() {
    String html = "<html><head><meta http-equiv='refresh' content='5'/><title>TEROS 12 Sensor Data Log</title></head>";
    html += "<body><h1>TEROS 12 Sensor Readings Log</h1><pre id='log'>" + dataLog + "</pre>";
    html += "<script>setInterval(function(){fetch('/data').then(response => response.text()).then(data => {document.getElementById('log').innerHTML = data;});}, 5000);</script></body></html>";
    server.send(200, "text/html", html);
}

void handleData() {
    server.send(200, "text/plain", dataLog);
}

String getTimestamp() {
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
        return "Time not set";
    }
    char timeStringBuff[50];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return String(timeStringBuff);
}

void addToDataLog(String newEntry) {
    Serial.println("Adding to log: " + newEntry);  // Debug print
    dataLog = newEntry + "\n" + dataLog;
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

void setup_wifi() {
    Serial.println("\nStarting WiFi connection...");
    Serial.print("Connecting to: ");
    Serial.println(ssid);
    
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    WiFi.begin(ssid, password);
    
    int attempt = 0;
    while (WiFi.status() != WL_CONNECTED && attempt < 30) {
        delay(1000);
        Serial.print(".");
        attempt++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected successfully");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("Signal Strength (RSSI): ");
        Serial.println(WiFi.RSSI());
    } else {
        Serial.println("\nWiFi connection failed! Will retry in loop...");
    }
}

void reconnectMQTT() {
    if (!client.connected()) {
        Serial.print("Attempting MQTT connection... ");
        if (client.connect(mqtt_client_id, mqtt_user, mqtt_password, availabilityTopic.c_str(), 1, true, "offline")) {
            Serial.println("connected");
            client.publish(availabilityTopic.c_str(), "online", true);
            publishDiscoveryConfig();
        } else {
            Serial.print("failed, rc=");
            Serial.println(client.state());
        }
    }
}

void publishDiscoveryConfig() {
    Serial.println("Publishing discovery config...");
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
        {"raw_vwc", "Raw VWC", "", nullptr, "{{ value_json.raw_vwc|round(2) }}"}
    };
    
    for (const auto& sensor : sensors) {
        String discoveryTopic = String(MQTT_TOPIC_BASE) + "/" + device_name + "/" + sensor.type + "/config";
        DynamicJsonDocument doc(512);
        char buffer[512];
        
        doc["name"] = String(device_name) + " " + sensor.name;
        doc["unique_id"] = String(device_name) + "_" + sensor.type;
        doc["state_topic"] = stateTopic;
        doc["availability_topic"] = availabilityTopic;
        doc["value_template"] = sensor.value_template;
        
        if (sensor.unit) doc["unit_of_measurement"] = sensor.unit;
        if (sensor.device_class) doc["device_class"] = sensor.device_class;
        
        doc["state_class"] = "measurement";
        
        JsonObject device = doc.createNestedObject("device");
        device["identifiers"][0] = device_name;
        device["manufacturer"] = MANUFACTURER;
        device["model"] = MODEL;
        device["name"] = device_name;
        device["sw_version"] = SW_VERSION;
        
        serializeJson(doc, buffer);
        Serial.println("Publishing config for " + String(sensor.type));
        client.publish(discoveryTopic.c_str(), buffer, true);
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000);  // Give serial time to start
    
    Serial.println("\n\n=== TEROS-12 Sensor Starting ===");
    Serial.println("Initializing SDI-12...");
    sdi12.begin();
    
    if (sdi12.ackActive(DEVICE_ADDRESS) == ESP32_SDI12::SDI12_OK) {
        Serial.println("SDI-12 sensor responding");
    } else {
        Serial.println("WARNING: SDI-12 sensor not responding!");
    }
    
    setup_wifi();
    
    Serial.println("Setting up MQTT...");
    snprintf(mqtt_client_id, sizeof(mqtt_client_id), "ESP32Client-%s", device_name);
    stateTopic = String(MQTT_TOPIC_BASE) + "/" + device_name + "/state";
    availabilityTopic = String(MQTT_TOPIC_BASE) + "/" + device_name + "/availability";
    errorTopic = String(MQTT_TOPIC_BASE) + "/" + device_name + "/error";
    
    Serial.println("MQTT Topics:");
    Serial.println("State: " + stateTopic);
    Serial.println("Availability: " + availabilityTopic);
    Serial.println("Error: " + errorTopic);
    
    client.setServer(mqtt_server, mqtt_port);
    
    Serial.println("Starting web server...");
    server.on("/", HTTP_GET, handleRoot);
    server.on("/data", HTTP_GET, handleData);
    server.begin();
    
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    Serial.println("Setup complete\n");
}

void loop() {
    static unsigned long lastMsg = 0;
    
    // Check WiFi connection
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi disconnected. Reconnecting...");
        setup_wifi();
        return;  // Skip this loop iteration
    }
    
    server.handleClient();
    
    if (!client.connected()) {
        unsigned long now = millis();
        if (now - lastReconnectAttempt > mqttReconnectInterval) {
            lastReconnectAttempt = now;
            reconnectMQTT();
        }
    } else {
        client.loop();
    }

    unsigned long now = millis();
    if (now - lastMsg > READING_INTERVAL) {
        lastMsg = now;
        
        Serial.println("\n=== Taking Sensor Reading ===");
        ESP32_SDI12::Status res = sdi12.measure(DEVICE_ADDRESS, values, VALUES_BUFFER_SIZE);
        
        if (res == ESP32_SDI12::SDI12_OK) {
            Serial.println("Raw sensor values:");
            readings.raw_vwc = values[0];
            readings.temperature = values[1];
            readings.bulk_ec = values[2] / 1000.0;

            Serial.printf("Raw VWC: %.2f\n", readings.raw_vwc);
            Serial.printf("Temperature: %.2f°C\n", readings.temperature);
            Serial.printf("Bulk EC: %.3f dS/m\n", readings.bulk_ec);

            readings.vwc = calculateVWC(readings.raw_vwc);
            float bulk_permittivity = calculateBulkPermittivity(readings.raw_vwc);
            readings.temp_compensated_ec = compensateECForTemperature(readings.bulk_ec, readings.temperature);
            
            if (readings.vwc >= 0.10) {
                readings.pore_water_ec = calculatePoreWaterEC(readings.temp_compensated_ec, readings.temperature, bulk_permittivity);
                readings.saturation_extract_ec = calculateSaturationExtractEC(readings.pore_water_ec, readings.vwc);
            } else {
                readings.pore_water_ec = 0;
                readings.saturation_extract_ec = 0;
                Serial.println("VWC too low for EC calculations");
            }

            String logEntry = getTimestamp() + ": Raw VWC: " + String(readings.raw_vwc, 2) + 
                            ", VWC: " + String(readings.vwc * 100, 2) + "%, Temp: " + 
                            String(readings.temperature, 2) + " C, Bulk EC: " + 
                            String(readings.bulk_ec, 3) + " dS/m";
            addToDataLog(logEntry);

            DynamicJsonDocument doc(256);
            char buffer[256];
            
            doc["raw_vwc"] = readings.raw_vwc;
            doc["vwc"] = readings.vwc * 100;
            doc["temperature"] = readings.temperature;
            doc["bulk_ec"] = readings.bulk_ec;
            doc["temp_comp_ec"] = readings.temp_compensated_ec;
            doc["pore_water_ec"] = readings.pore_water_ec;
            doc["saturation_extract_ec"] = readings.saturation_extract_ec;
            
            serializeJson(doc, buffer);
            Serial.println("Publishing to MQTT: " + String(buffer));
            client.publish(stateTopic.c_str(), buffer, true);
        } else {
            Serial.printf("Sensor read failed with error: %d\n", res);
            client.publish(errorTopic.c_str(), "Sensor read failed", true);
        }
    }
}
