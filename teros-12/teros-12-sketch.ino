#include <WiFi.h>
#include <PubSubClient.h>
#include <esp32-sdi12.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <time.h>

// Configuration Section
const char* ssid = "Wifi AP";                      // WiFi SSID
const char* password = "Wifi AP Password";                      // WiFi Password
const char* mqtt_server = "192.168.50.196";             // MQTT Broker IP
const int mqtt_port = 1883;                             // MQTT Broker Port
const char* mqtt_user = "mqtt";                         // MQTT Username
const char* mqtt_password = "ttqm";                     // MQTT Password
const char* device_name = "teros-usa-1-5";              // Unique device name for MQTT client ID
const char* mqtt_topic = "sdi12/teros-usa-1-5";         // Unique MQTT Topic for publishing data
const uint8_t DEVICE_ADDRESS = 0;                       // SDI-12 Address of the device, ensure unique if multiple devices on same line
#define SDI12_DATA_PIN 26                               // GPIO pin for SDI-12 data line

// Rockwool specific constants
const float ROCKWOOL_TOTAL_POROSITY = 0.95;             // Total porosity of the rockwool, adjust based on your specific type
const float OFFSET_PERMITTIVITY = 4.1;                  // Offset permittivity for rockwool, might need adjustment for accuracy

// Buffer size for SDI-12 sensor measurements
#define VALUES_BUFFER_SIZE 10                           // Size of the buffer to hold values from a measurement

// Maximum number of log entries to keep
const int MAX_LOG_ENTRIES = 20;

// MQTT reconnection interval (in milliseconds)
const unsigned long mqttReconnectInterval = 5000;       // 5 seconds between connection attempts

char mqtt_client_id[50];

ESP32_SDI12 sdi12(SDI12_DATA_PIN);
float values[VALUES_BUFFER_SIZE]; // Buffer to hold values from a measurement

WiFiClient espClient;
PubSubClient client(espClient);
WebServer server(80);

// Global variables to store sensor readings
float raw_vwc, vwc, temperature, bulk_ec, temp_compensated_ec, pore_water_ec, saturation_extract_ec;

// Global variable to store log data
String dataLog = "";

unsigned long lastReconnectAttempt = 0;

void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void reconnect() {
    if (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        snprintf(mqtt_client_id, sizeof(mqtt_client_id), "ESP32Client-%s", device_name);
        if (client.connect(mqtt_client_id, mqtt_user, mqtt_password)) {
            Serial.println("connected");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
        }
    }
}

void setup() {
    Serial.begin(115200);
    sdi12.begin();
    setup_wifi();

    // Set unique MQTT client ID
    snprintf(mqtt_client_id, sizeof(mqtt_client_id), "ESP32Client-%s", device_name);
    
    client.setServer(mqtt_server, mqtt_port);
    
    server.on("/", HTTP_GET, handleRoot);
    server.on("/data", HTTP_GET, handleData);
    server.begin();
    Serial.println("HTTP server started");
    
    // Initialize time
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
    html += "<meta http-equiv='refresh' content='5'/>";  // Refresh every 5 seconds
    html += "<title>TEROS 12 Sensor Data Log</title></head>";
    html += "<body><h1>TEROS 12 Sensor Readings Log</h1>";
    html += "<pre id='log'>" + dataLog + "</pre>";
    html += "<script>";
    html += "setInterval(function() {";
    html += "  fetch('/data')";
    html += "    .then(response => response.text())";
    html += "    .then(data => {";
    html += "      document.getElementById('log').innerHTML = data;";
    html += "    });";
    html += "}, 5000);";  // Update every 5 seconds
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
    
    if (!client.connected()) {
        unsigned long now = millis();
        if (now - lastReconnectAttempt > mqttReconnectInterval) {
            lastReconnectAttempt = now;
            reconnect();
        }
    } else {
        client.loop();
    }

    unsigned long currentMillis = millis();
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

            // Create log entry with timestamp
            String logEntry = getTimestamp() + ": " +
                              "Raw VWC: " + String(raw_vwc, 2) + ", " +
                              "VWC: " + String(vwc * 100, 2) + "%, " +
                              "Temp: " + String(temperature, 2) + " C, " +
                              "Bulk EC: " + String(bulk_ec, 3) + " dS/m, " +
                              "Temp Comp EC: " + String(temp_compensated_ec, 3) + " dS/m, " +
                              "Pore Water EC: " + String(pore_water_ec, 3) + " dS/m, " +
                              "Sat. Ext. EC: " + String(saturation_extract_ec, 3) + " dS/m";
            addToDataLog(logEntry);

            // Print values for debugging
            Serial.println(logEntry);

            // Create JSON payload for MQTT
            String payload = "{\"raw_vwc\": " + String(raw_vwc) + 
                             ", \"vwc\": " + String(vwc * 100) + 
                             ", \"temperature\": " + String(temperature) + 
                             ", \"bulk_ec\": " + String(bulk_ec) + 
                             ", \"temp_comp_ec\": " + String(temp_compensated_ec) + 
                             ", \"pore_water_ec\": " + String(pore_water_ec) + 
                             ", \"saturation_extract_ec\": " + String(saturation_extract_ec) + "}";
            Serial.print("Publishing payload: ");
            Serial.println(payload); // Debug MQTT payload
            if (client.publish(mqtt_topic, payload.c_str())) {
                Serial.println("Publish succeeded");
            } else {
                Serial.println("Publish failed");
            }
        }
    }
}
