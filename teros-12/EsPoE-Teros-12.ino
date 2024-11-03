#include <Arduino.h>
#include <esp32-sdi12.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <time.h>
#include "Unit_PoESP32.h"

// Configuration Section
const char* mqtt_server = "192.168.73.250";       // MQTT Broker IP
const char* mqtt_port = "1883";                   // MQTT Broker Port as string
const char* mqtt_user = "mqtt";                   // MQTT Username
const char* mqtt_password = "ttqm";               // MQTT Password
const char* device_name = "tc-veg-2";             // Base device name for MQTT client ID
const char* mqtt_topic = "homeassistant/sdi12/tc-veg-2"; // Unique MQTT Topic for publishing data
const uint8_t DEVICE_ADDRESS = 0;                 // SDI-12 Address of the device
#define SDI12_DATA_PIN 16                         // Changed to pin 16 for PoESP32

// Rockwool specific constants
const float ROCKWOOL_TOTAL_POROSITY = 0.95;       // Total porosity of the rockwool
const float OFFSET_PERMITTIVITY = 4.1;            // Offset permittivity for rockwool

// Buffer size for SDI-12 sensor measurements
#define VALUES_BUFFER_SIZE 10                     // Size of the buffer to hold values from a measurement

// Maximum number of log entries to keep
const int MAX_LOG_ENTRIES = 20;

Unit_PoESP32 eth;
ESP32_SDI12 sdi12(SDI12_DATA_PIN);
float values[VALUES_BUFFER_SIZE]; // Buffer to hold values from a measurement
WebServer server(80);

// Global variables to store sensor readings
float raw_vwc, vwc, temperature, bulk_ec, temp_compensated_ec, pore_water_ec, saturation_extract_ec;

// Log data buffer
char dataLog[2048]; // Adjust size as needed
int logEntryCount = 0;

// Timestamp buffer
char timestamp[50];

typedef enum { kConfig = 0, kStart, kConnecting, kConnected } DTUState_t;
DTUState_t State = kStart;

void setup() {
    Serial.begin(115200);
    sdi12.begin();
    
    // Initialize PoESP32
    eth.Init(&Serial2, 9600, 32, 26);
    
    Serial.println("Waiting for device connection...");
    while (!eth.checkDeviceConnect()) {
        delay(100);
    }
    Serial.println("Device connected");

    Serial.println("Waiting for ethernet connection...");
    while (!eth.checkETHConnect()) {
        delay(100);
    }
    Serial.println("Ethernet connected");

    // Create unique client ID using device name
    String clientId = String("ESP32Client-") + String(device_name) + String("-") + 
                     String((uint32_t)ESP.getEfuseMac(), HEX);

    Serial.println("Connecting to MQTT...");
    while (!eth.createMQTTClient(mqtt_server, mqtt_port, clientId.c_str(),
                                mqtt_user, mqtt_password)) {
        delay(100);
    }
    Serial.println("MQTT connected");

    server.on("/", HTTP_GET, handleRoot);
    server.on("/data", HTTP_GET, handleData);
    server.begin();
    Serial.println("HTTP server started");

    // Initialize time
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    // Initialize dataLog
    dataLog[0] = '\0';

    State = kConnected;
}

// Keep your existing calculation functions unchanged
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

// Keep your existing web handler functions unchanged
void handleRoot() {
    String html = "<html><head>";
    html += "<meta http-equiv='refresh' content='5'/>";
    html += "<title>TC-Veg-2 Sensor Data Log</title></head>";  // Changed to tc-veg-2
    html += "<body><h1>TC-Veg-2 Sensor Readings Log</h1>";    // Changed to tc-veg-2
    html += "<pre id='log'>" + String(dataLog) + "</pre>";
    html += "<script>";
    html += "setInterval(function() {";
    html += "  fetch('/data')";
    html += "    .then(response => response.text())";
    html += "    .then(data => {";
    html += "      document.getElementById('log').innerHTML = data;";
    html += "    });";
    html += "}, 5000);";
    html += "</script></body></html>";
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

    static unsigned long lastReadingTime = 0;
    unsigned long currentMillis = millis();
    
    if (currentMillis - lastReadingTime >= 15000) { // 15 seconds
        lastReadingTime = currentMillis;

        ESP32_SDI12::Status res = sdi12.measure(DEVICE_ADDRESS, values, sizeof(values) / sizeof(values[0]));

        if (res != ESP32_SDI12::SDI12_OK) {
            Serial.printf("Error: %d\n", res);
        } else {
            raw_vwc = values[0];
            vwc = calculateVWC(raw_vwc);
            temperature = values[1];
            bulk_ec = values[2] / 1000.0; // Convert µS/cm to dS/m

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

            char logEntry[256];
            snprintf(logEntry, sizeof(logEntry), "%s: Raw VWC: %.2f, VWC: %.2f%%, Temp: %.2f C, Bulk EC: %.3f dS/m, Temp Comp EC: %.3f dS/m, Pore Water EC: %.3f dS/m, Sat. Ext. EC: %.3f dS/m",
                     timestamp, raw_vwc, vwc * 100, temperature, bulk_ec, temp_compensated_ec, pore_water_ec, saturation_extract_ec);
            addToDataLog(logEntry);

            Serial.println(logEntry);

            // Create JSON payload
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

            // Publish to MQTT using PoESP32
            if (!eth.publicMQTTMsg(mqtt_topic, payload, "1")) {
                Serial.println("Failed to publish MQTT message");
            } else {
                Serial.println("MQTT message published successfully");
            }
        }
    }
}
