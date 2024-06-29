#include <WiFi.h>
#include <PubSubClient.h>
#include <esp32-sdi12.h> # available here https://github.com/HarveyBates/ESP32-SDI12

// WiFi credentials
const char* ssid = "Wifi 420 69";
const char* password = "42069420";

// MQTT broker information
const char* mqtt_server = "192.168.50.196";
const int mqtt_port = 1883;
const char* mqtt_user = "mqtt";
const char* mqtt_password = "ttqm";
const char* mqtt_topic = "sdi12/teros-12";

#define SDI12_DATA_PIN 16 // Change based on the pin you are using on your esp
#define DEVICE_ADDRESS uint8_t(0) // SDI-12 Address of device

ESP32_SDI12 sdi12(SDI12_DATA_PIN);
float values[10]; // Buffer to hold values from a measurement

WiFiClient espClient;
PubSubClient client(espClient);

// Calibration factor for EC (assuming correct value is double the current reading)
const float EC_CALIBRATION_FACTOR = 1.0;

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
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
            Serial.println("connected");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);

    // Initialize SDI-12 pin definition
    sdi12.begin();

    // Connect to WiFi
    setup_wifi();

    // Set MQTT server
    client.setServer(mqtt_server, mqtt_port);

    // Initial delay to stabilize
    delay(1000);
}

// Function to calculate VWC for non-soil substrates
float calculateVWC(float raw) {
    return (6.771e-10 * pow(raw, 3) - 5.105e-6 * pow(raw, 2) + 1.302e-2 * raw - 10.848);
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    // Measure (will populate values array with data)
    ESP32_SDI12::Status res = sdi12.measure(DEVICE_ADDRESS, values, sizeof(values) / sizeof(values[0]));
    
    if (res != ESP32_SDI12::SDI12_OK) {
        Serial.printf("Error: %d\n", res);
    } else {
        // Assuming values[0] is Raw VWC, values[1] is Temperature, values[2] is EC
        float raw_vwc = values[0];
        float vwc = calculateVWC(raw_vwc) * 100; // VWC calculation and then multiply by 100
        float temperature = values[1]; // Temperature as is since it's correct (in °C)
        float ec = (values[2] / 1000.0) * EC_CALIBRATION_FACTOR; // EC in µS/cm divided by 1000 to get dS/m and then apply calibration factor

        // Print values for debugging
        Serial.printf("VWC: %f%%, Temperature: %f°C, EC: %f dS/m\n", vwc, temperature, ec);

        // Create JSON payload
        String payload = "{\"raw\": " + String(raw_vwc) + ", \"vwc\": " + String(vwc) + 
                         ", \"temperature\": " + String(temperature) + ", \"ec\": " + String(ec) + "}";
        client.publish(mqtt_topic, payload.c_str());
    }

    delay(15000); // Do this measurement every 15 seconds
}
