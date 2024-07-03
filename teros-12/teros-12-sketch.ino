#include <WiFi.h>
#include <PubSubClient.h>
#include <esp32-sdi12.h>
#include <ArduinoOTA.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <time.h>

// WiFi credentials
const char* ssid = "Wifi AP Name";  //// UPDATE THIS
const char* password = "Wifi AP PW"; //// UPDATE THIS

// MQTT broker information
const char* mqtt_server = "192.168.50.196"; //// UPDATE THIS
const int mqtt_port = 1883;
const char* mqtt_user = "mqtt"; //// UPDATE THIS
const char* mqtt_password = "ttqm"; //// UPDATE THIS
const char* mqtt_topic = "sdi12/mtec-w2"; //// UPDATE THIS

#define SDI12_DATA_PIN 2 // UPDATE THIS to the number on your ESP32 device 
#define DEVICE_ADDRESS uint8_t(0) // SDI-12 Address of device // THIS stays the same for the Chinese and legit versions

ESP32_SDI12 sdi12(SDI12_DATA_PIN);
float values[10]; // Buffer to hold values from a measurement

WiFiClient espClient;
PubSubClient client(espClient);
WebServer server(80);

// Rockwool specific constants
const float ROCKWOOL_TOTAL_POROSITY = 0.95; // Adjust this value based on your specific rockwool type
const float OFFSET_PERMITTIVITY = 4.1; // This might need adjustment for rockwool

// Global variables to store sensor readings
float raw_vwc, vwc, temperature, bulk_ec, temp_compensated_ec, pore_water_ec, saturation_extract_ec;

// New global variable to store log data
String dataLog = "";
const int MAX_LOG_ENTRIES = 20;  // Maximum number of log entries to keep

unsigned long lastMqttPublish = 0;
const unsigned long mqttPublishInterval = 15000;  // Publish every 15 seconds

const int mqttKeepAlive = 60;  // MQTT keep-alive interval in seconds
unsigned long lastMqttConnectionAttempt = 0;
const unsigned long mqttConnectionInterval = 5000;  // 5 seconds between connection attempts

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

void setup_ota() {
    ArduinoOTA.setHostname("ESP32-TEROS12");
    ArduinoOTA.setPassword("your_ota_password"); // Set a password for OTA updates

    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_SPIFFS
            type = "filesystem";
        }
        Serial.println("Start updating " + type);
    });
    
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    
    ArduinoOTA.begin();
    Serial.println("OTA Ready");
}

boolean mqttConnect() {
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
        Serial.println("MQTT connected");
        return true;
    } else {
        Serial.print("MQTT connection failed, rc=");
        Serial.println(client.state());
        return false;
    }
}

void setup() {
    Serial.begin(115200);
    sdi12.begin();
    setup_wifi();
    setup_ota();
    client.setServer(mqtt_server, mqtt_port);
    client.setKeepAlive(mqttKeepAlive);
    
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
    ArduinoOTA.handle();
    server.handleClient();
    
    unsigned long currentMillis = millis();

    // MQTT connection management
    if (!client.connected()) {
        if (currentMillis - lastMqttConnectionAttempt >= mqttConnectionInterval) {
            lastMqttConnectionAttempt = currentMillis;
            if (mqttConnect()) {
                lastMqttConnectionAttempt = 0;  // Reset the timer if connection successful
            }
        }
    } else {
        client.loop();
    }

    // Sensor reading and MQTT publishing
    if (currentMillis - lastMqttPublish >= mqttPublishInterval) {
        lastMqttPublish = currentMillis;
        
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
            
            if (client.connected()) {
                if (client.publish(mqtt_topic, payload.c_str())) {
                    Serial.println("MQTT message published successfully");
                } else {
                    Serial.println("MQTT message publish failed");
                }
            } else {
                Serial.println("MQTT not connected. Message not published.");
            }
        }
    }
}
