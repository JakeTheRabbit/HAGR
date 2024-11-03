#include <Arduino.h>
#include <esp32-sdi12.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <time.h>
#include "Unit_PoESP32.h"

// Pin Definitions
#define POE_RX 16        // PoESP32 RX pin
#define POE_TX 17        // PoESP32 TX pin
#define POE_RST 5        // PoESP32 Reset pin
#define SDI12_DATA_PIN 26 // SDI-12 data pin

// MQTT Configuration
const char* mqtt_server = "192.168.73.250";
const char* mqtt_port = "1883";
const char* mqtt_user = "mqtt";
const char* mqtt_password = "ttqm";
const char* device_name = "tc-veg-2";
const char* mqtt_topic = "homeassistant/sdi12/tc-veg-2";

// Device Configuration
const uint8_t DEVICE_ADDRESS = 0;
#define VALUES_BUFFER_SIZE 10
#define MAX_LOG_ENTRIES 20

// Constants
const float ROCKWOOL_TOTAL_POROSITY = 0.95;
const float OFFSET_PERMITTIVITY = 4.1;

// Global Objects
Unit_PoESP32 eth;
ESP32_SDI12 sdi12(SDI12_DATA_PIN);
float values[VALUES_BUFFER_SIZE];
WebServer server(80);

// Global Variables
float raw_vwc, vwc, temperature, bulk_ec, temp_compensated_ec, pore_water_ec, saturation_extract_ec;
char dataLog[2048];
char timestamp[50];
int logEntryCount = 0;

typedef enum { kConfig = 0, kStart, kConnecting, kConnected } DTUState_t;
DTUState_t State = kStart;

void testATCommands() {
    Serial.println("\nTesting AT commands:");
    
    // Basic AT test
    Serial.println("Sending AT...");
    eth.sendCMD("AT");
    delay(1000);
    String response = eth.waitMsg(1000);
    Serial.print("Response: ");
    Serial.println(response);
    
    // Version information
    Serial.println("Sending AT+GMR...");
    eth.sendCMD("AT+GMR");
    delay(1000);
    response = eth.waitMsg(1000);
    Serial.print("Response: ");
    Serial.println(response);
    
    // Ethernet status
    Serial.println("Sending AT+CIPETH?...");
    eth.sendCMD("AT+CIPETH?");
    delay(1000);
    response = eth.waitMsg(1000);
    Serial.print("Response: ");
    Serial.println(response);
}

void setup() {
    disableCore0WDT();
    
    // Initialize debug serial
    Serial.begin(115200);
    while(!Serial) {
        delay(100);
    }
    Serial.println("\nStarting up...");
    
    // Initialize SDI-12
    sdi12.begin();
    
    // Reset PoESP32
    pinMode(POE_RST, OUTPUT);
    digitalWrite(POE_RST, LOW);
    delay(100);
    digitalWrite(POE_RST, HIGH);
    delay(500);
    
    // Initialize Serial2 for PoESP32 at default baud rate
    Serial2.begin(9600, SERIAL_8N1, POE_RX, POE_TX);
    delay(1000);
    
    Serial.println("Initializing PoESP32...");
    eth.Init(&Serial2, 9600, POE_RX, POE_TX);
    delay(2000);
    
    // Clear any pending data
    while(Serial2.available()) {
        Serial2.read();
    }
    
    // Test AT commands
    testATCommands();
    
    Serial.println("Waiting for device connection...");
    int retries = 0;
    while (!eth.checkDeviceConnect()) {
        delay(1000);
        Serial.print(".");
        retries++;
        if (retries > 10) {
            Serial.println("\nRetrying device initialization...");
            eth.Init(&Serial2, 9600, POE_RX, POE_TX);
            retries = 0;
        }
    }
    Serial.println("\nDevice connected");

    Serial.println("Waiting for ethernet connection...");
    retries = 0;
    while (!eth.checkETHConnect()) {
        delay(1000);
        Serial.print(".");
        retries++;
        if (retries > 10) {
            Serial.println("\nRetrying ethernet connection...");
            retries = 0;
        }
    }
    Serial.println("\nEthernet connected");

    // Create unique client ID
    String clientId = String("ESP32Client-") + String(device_name) + String("-") + 
                     String((uint32_t)ESP.getEfuseMac(), HEX);
    Serial.print("MQTT Client ID: ");
    Serial.println(clientId);
    
    // Connect to MQTT
    Serial.println("Connecting to MQTT...");
    Serial.printf("Server: %s, Port: %s\n", mqtt_server, mqtt_port);
    
    retries = 0;
    bool mqtt_connected = false;
    while (!mqtt_connected && retries < 5) {
        Serial.printf("MQTT connection attempt %d...\n", retries + 1);
        if (eth.createMQTTClient(mqtt_server, mqtt_port, clientId.c_str(),
                                mqtt_user, mqtt_password)) {
            mqtt_connected = true;
            Serial.println("MQTT connected successfully!");
            
            // Subscribe to command topic
            if (eth.subscribeMQTTMsg("homeassistant/sdi12/tc-veg-2/command", "1")) {
                Serial.println("Successfully subscribed to command topic");
            } else {
                Serial.println("Failed to subscribe to command topic");
            }
        } else {
            Serial.println("Failed to connect to MQTT");
            delay(5000);
            retries++;
        }
    }

    if (!mqtt_connected) {
        Serial.println("Failed to connect to MQTT after multiple attempts");
        ESP.restart();
    }

    // Setup web server
    server.on("/", HTTP_GET, handleRoot);
    server.on("/data", HTTP_GET, handleData);
    server.begin();
    Serial.println("HTTP server started");

    // Initialize time
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    dataLog[0] = '\0';
    State = kConnected;
}

// Your existing calculation functions remain unchanged
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

// Web server handlers
void handleRoot() {
    String html = "<html><head>";
    html += "<meta http-equiv='refresh' content='5'/>";
    html += "<title>TC-Veg-2 Sensor Data Log</title></head>";
    html += "<body><h1>TC-Veg-2 Sensor Readings Log</h1>";
    html += "<pre id='log'>" + String(dataLog) + "</pre>";
    html += "</body></html>";
    server.send(200, "text/html", html);
}

void handleData() {
    server.send(200, "text/plain", String(dataLog));
}

void getTimestamp() {
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
        strcpy(timestamp, "Time not set");
        return;
    }
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);
}

void addToDataLog(const char* newEntry) {
    char tempLog[2048];
    snprintf(tempLog, sizeof(tempLog), "%s\n%s", newEntry, dataLog);
    strncpy(dataLog, tempLog, sizeof(dataLog) - 1);
    dataLog[sizeof(dataLog) - 1] = '\0';

    int newlineCount = 0;
    for (int i = 0; i < strlen(dataLog); i++) {
        if (dataLog[i] == '\n') {
            newlineCount++;
            if (newlineCount > MAX_LOG_ENTRIES) {
                dataLog[i] = '\0';
                break;
            }
        }
    }
}

void loop() {
    server.handleClient();

    static unsigned long lastMQTTCheck = 0;
    static unsigned long lastReadingTime = 0;
    static unsigned long lastTestMessage = 0;
    unsigned long currentMillis = millis();
    
    // Send test message every 10 seconds
    if (currentMillis - lastTestMessage >= 10000) {
        lastTestMessage = currentMillis;
        if (!eth.publicMQTTMsg("homeassistant/sdi12/tc-veg-2/test", "test_message", "1")) {
            Serial.println("Test message failed to send");
        } else {
            Serial.println("Test message sent successfully");
        }
    }
    
    // Check MQTT connection every 30 seconds
    if (currentMillis - lastMQTTCheck >= 30000) {
        lastMQTTCheck = currentMillis;
        if (!eth.publicMQTTMsg("homeassistant/sdi12/tc-veg-2/heartbeat", "alive", "0")) {
            Serial.println("Heartbeat failed, attempting to reconnect MQTT...");
            String clientId = String("ESP32Client-") + String(device_name) + String("-") + 
                            String((uint32_t)ESP.getEfuseMac(), HEX);
            if (eth.createMQTTClient(mqtt_server, mqtt_port, clientId.c_str(),
                                   mqtt_user, mqtt_password)) {
                Serial.println("MQTT reconnected successfully");
            } else {
                Serial.println("MQTT reconnection failed");
            }
        }
    }
    
    // Take sensor readings every 15 seconds
    if (currentMillis - lastReadingTime >= 15000) {
        lastReadingTime = currentMillis;
        
        ESP32_SDI12::Status res = sdi12.measure(DEVICE_ADDRESS, values, sizeof(values) / sizeof(values[0]));

        if (res != ESP32_SDI12::SDI12_OK) {
            Serial.printf("Error: %d\n", res);
        } else {
            // Process sensor readings
            raw_vwc = values[0];
            vwc = calculateVWC(raw_vwc);
            temperature = values[1];
            bulk_ec = values[2] / 1000.0;

            float bulk_permittivity = calculateBulkPermittivity(raw_vwc);
            temp_compensated_ec = compensateECForTemperature(bulk_ec, temperature);

            if (vwc >= 0.10) {
                pore_water_ec = calculatePoreWaterEC(temp_compensated_ec, temperature, bulk_permittivity);
                saturation_extract_ec = calculateSaturationExtractEC(pore_water_ec, vwc);
            } else {
                Serial.println("Warning: VWC too low for accurate EC calculation");
                pore_water_ec = 0;
                saturation_extract_ec = 0;
            }

            getTimestamp();

            // Create log entry
            char logEntry[256];
            snprintf(logEntry, sizeof(logEntry), "%s: Raw VWC: %.2f, VWC: %.2f%%, Temp: %.2f C, Bulk EC: %.3f dS/m, Temp Comp EC: %.3f dS/m, Pore Water EC: %.3f dS/m, Sat. Ext. EC: %.3f dS/m",
                     timestamp, raw_vwc, vwc * 100, temperature, bulk_ec, temp_compensated_ec, pore_water_ec, saturation_extract_ec);
            addToDataLog(logEntry);

            Serial.println(logEntry);

            // Create and publish MQTT message
            StaticJsonDocument<256> doc;
            doc["raw_vwc"] = raw_vwc;
            doc["vwc"] = vwc * 100;
            doc["temperature"] = temperature;
            doc["bulk_ec"] = bulk_ec;
            doc["temp_comp_ec"] = temp_compensated_ec;
            doc["pore_water_ec"] = pore_water_ec;
            doc["saturation_extract_ec"] = saturation_extract_ec;

            char payload[256];
            serializeJson(doc, payload, sizeof(payload));

            Serial.println("Publishing MQTT message...");
            Serial.print("Topic: ");
            Serial.println(mqtt_topic);
            Serial.print("Payload: ");
            Serial.println(payload);
            
            bool published = false;
            for(int i = 0; i < 3 && !published; i++) {
                if (eth.publicMQTTMsg(mqtt_topic, payload, "1")) {
                    Serial.println("MQTT message published successfully");
                    published = true;
                } else {
                    Serial.printf("Failed to publish MQTT message, attempt %d\n", i + 1);
                    delay(1000);
                }
            }
            
            if (!published) {
                Serial.println("Failed to publish after all attempts, reconnecting MQTT...");
                String clientId = String("ESP32Client-") + String(device_name) + String("-") + 
                                String((uint32_t)ESP.getEfuseMac(), HEX);
                if (eth.createMQTTClient(mqtt_server, mqtt_port, clientId.c_str(),
                                       mqtt_user, mqtt_password)) {
                    Serial.println("MQTT reconnected successfully");
                } else {
                    Serial.println("MQTT reconnection failed");
                }
            }
        }
    }
}
