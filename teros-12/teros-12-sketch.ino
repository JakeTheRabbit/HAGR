#include <WiFi.h>
#include <PubSubClient.h>
#include <esp32-sdi12.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <time.h>

// Configuration
const char* ssid = "FreeSpacesF5";                      // WiFi SSID
const char* password = "p0lyview";                      // WiFi Password
const char* mqtt_server = "192.168.50.196";             // MQTT Broker IP
const int mqtt_port = 1883;                             // MQTT Broker Port
const char* mqtt_user = "mqtt";                         // MQTT Username
const char* mqtt_password = "ttqm";                     // MQTT Password
const char* device_name = "teros-usa-1-5";              // Unique device name
const char* mqtt_topic = "sdi12/teros-usa-1-5";         // Unique MQTT Topic
const uint8_t DEVICE_ADDRESS = 0;                       // SDI-12 Address of the device, ensure unique if multiple devices on same line

#define SDI12_DATA_PIN 26 // Change based on the pin you are using

char mqtt_client_id[50];

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

unsigned long lastReconnectAttempt = 0;
const unsigned long mqttReconnectInterval = 5000;  // 5 seconds between connection attempts

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
    
    client.setServer
