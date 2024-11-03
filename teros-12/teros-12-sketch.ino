#include <Arduino.h>
#include <esp32-sdi12.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <time.h>
#include "Unit_PoESP32.h"

// Configuration Section
const char* mqtt_server = "192.168.73.250";       
const char* mqtt_port = "1883";                   
const char* mqtt_user = "mqtt";                   
const char* mqtt_password = "ttqm";               
const char* device_name = "tc-veg-2";             
const char* mqtt_topic = "sdi12/tc-veg-2";        
const uint8_t DEVICE_ADDRESS = 0;                 
#define SDI12_DATA_PIN 16                         

// Rockwool specific constants
const float ROCKWOOL_TOTAL_POROSITY = 0.95;       
const float OFFSET_PERMITTIVITY = 4.1;            

// Buffer size for SDI-12 sensor measurements
#define VALUES_BUFFER_SIZE 10                     

// Maximum number of log entries to keep
const int MAX_LOG_ENTRIES = 20;

// MQTT reconnection interval (in milliseconds)
const unsigned long mqttReconnectInterval = 5000; 

Unit_PoESP32 eth;
ESP32_SDI12 sdi12(SDI12_DATA_PIN);
float values[VALUES_BUFFER_SIZE];
WebServer server(80);

// Global variables to store sensor readings
float raw_vwc, vwc, temperature, bulk_ec, temp_compensated_ec, pore_water_ec, saturation_extract_ec;

// Global variable to store log data
String dataLog = "";

unsigned long lastReconnectAttempt = 0;

void setup() {
    Serial.begin(115200);
    sdi12.begin();
    
    // Initialize PoESP32
    eth.Init(&Serial2, 9600, 16, 17);
    delay(1000);
    
    Serial.println("Waiting for device connection...");
    while (!eth.checkDeviceConnect()) {
        delay(100);
        Serial.print(".");
    }
    Serial.println("\nDevice connected");

    Serial.println("Waiting for ethernet connection...");
    while (!eth.checkETHConnect()) {
        delay(100);
        Serial.print(".");
    }
    Serial.println("\nEthernet connected");

    // Create unique client ID
    String clientId = String("ESP32Client-") + String(device_name);
    
    Serial.println("Connecting to MQTT...");
    while (!eth.createMQTTClient(mqtt_server, mqtt_port, clientId.c_str(),
                                mqtt_user, mqtt_password)) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nMQTT connected");
    
    server.on("/", HTTP_GET, handleRoot);
    server.on("/data", HTTP_GET, handleData);
    server.begin();
    Serial.println("HTTP server started");
    
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    delay(1000);
}

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

void handleRoot() {
    String html = "<html><head>";
    html += "<meta http-equiv='refresh' content='5'/>";
    html += "<title>TC-Veg-2 Sensor Data Log</title></head>";
    html += "<body><h1>TC-Veg-2 Sensor Readings Log</h1>";
    html += "<pre id='log'>" + dataLog + "</pre>";
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

void loop() {
    server.handleClient();
    
    // Check MQTT connection every 30 seconds
    static unsigned long lastMQTTCheck = 0;
    unsigned long currentMillis = millis();
    
    if (currentMillis - lastMQTTCheck >= 30000) {
        lastMQTTCheck = currentMillis;
        if (!eth.publicMQTTMsg(mqtt_topic + String("/heartbeat"), "alive", "0")) {
            Serial.println("Lost MQTT connection, restarting...");
            ESP.restart();
        }
    }

    static unsigned long lastReadingTime = 0;
    if (currentMillis - lastReadingTime >= 15000) {
        lastReadingTime = currentMillis;
        
        ESP32_SDI12::Status res = sdi12.measure(DEVICE_ADDRESS, values, sizeof(values) / sizeof(values[0]));
        
        if (res != ESP32_SDI12::SDI12_OK) {
            Serial.printf("Error: %d\n", res);
        } else {
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

            String logEntry = getTimestamp() + ": " +
                              "Raw VWC: " + String(raw_vwc, 2) + ", " +
                              "VWC: " + String(vwc * 100, 2) + "%, " +
                              "Temp: " + String(temperature, 2) + " C, " +
                              "Bulk EC: " + String(bulk_ec, 3) + " dS/m, " +
                              "Temp Comp EC: " + String(temp_compensated_ec, 3) + " dS/m, " +
                              "Pore Water EC: " + String(pore_water_ec, 3) + " dS/m, " +
                              "Sat. Ext. EC: " + String(saturation_extract_ec, 3) + " dS/m";
            addToDataLog(logEntry);

            Serial.println(logEntry);

            String payload = "{\"raw_vwc\": " + String(raw_vwc) + 
                             ", \"vwc\": " + String(vwc * 100) + 
                             ", \"temperature\": " + String(temperature) + 
                             ", \"bulk_ec\": " + String(bulk_ec) + 
                             ", \"temp_comp_ec\": " + String(temp_compensated_ec) + 
                             ", \"pore_water_ec\": " + String(pore_water_ec) + 
                             ", \"saturation_extract_ec\": " + String(saturation_extract_ec) + "}";
            
            Serial.print("Publishing payload: ");
            Serial.println(payload);
            
            if (eth.publicMQTTMsg(mqtt_topic, payload.c_str(), "1")) {
                Serial.println("Publish succeeded");
            } else {
                Serial.println("Publish failed");
            }
        }
    }
}
