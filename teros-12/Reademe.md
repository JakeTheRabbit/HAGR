
I usually try and connect all my IoT devices through ESP Home to work with Home Assistant however the Arduino libraries for all the m5stack esp32 devices are far more comprehensive than the Esphome. Thankfully we have MQTT. 

This whole thing was made using Chat GPT/Claude I don't really have a working understanding so don't ask me for help. 

How to setup (this assumes you have basic knowledge of using Arduino IDE (ask Chat GPT for help its really good at it): 

1. Download Arduino IDE
2. Download https://github.com/HarveyBates/ESP32-SDI12 and install the library through zip in the arduino IDE. This enables SDI-12 communication directly with the ESP32.
3. Get an m5 stack grove cable and connect the cables see below image for what I did.
4. Get your ESP32 (I have tested with M5 Atom, M5 Atom S3, M5 PoEESP32 and M5 Dial).
5. Connect the Teros 12 to the ESP32 and plug into your computer
7. Copy the .ino file into the Arduino IDE and flash the ESP32. Change the #define SDI12_DATA_PIN 26 to whatever pin the yellow grove cable is connected to on your esp32.
8. Any errors paste into chat gpt or claude for trouble shooting
9. Profit?


If you have a Teros-12 Solus with the 3.5mm jack this is the pinout:

<img width="1109" alt="image" src="https://github.com/JakeTheRabbit/HAGR/assets/123831499/6e8107e2-4be7-4fe8-b744-577949a88612">
<img width="744" alt="image" src="https://github.com/JakeTheRabbit/HAGR/assets/123831499/767efecf-1174-4a88-aeeb-df83142dacce">

If you want to use the Chinese Teros-12 rip off that also works with this code for about 1/3 the price. https://www.alibaba.com/product-detail/China-low-price-CE-IP68-SID12_1600643601689.html

Wiring for connecting the Chinese Teros to the Grove cable: 

<img width="444" alt="image" src="https://github.com/JakeTheRabbit/HAGR/assets/123831499/e233d3d0-7c3c-494e-95d3-3f13e804ed6c">


Assumming you have mqtt setup in Home Assistant add this to your configuration.yaml file. 

```
mqtt:
  sensor:
    - name: "SDI12 Raw Sensor"
      state_topic: "sdi12/sensor"
      unit_of_measurement: "%"  # Replace with the appropriate unit if needed
      value_template: "{{ value_json.raw }}"

    - name: "SDI12 VWC Sensor"
      state_topic: "sdi12/sensor"
      unit_of_measurement: "%"
      value_template: "{{ value_json.vwc }}"

    - name: "SDI12 Temperature Sensor"
      state_topic: "sdi12/sensor"
      unit_of_measurement: "°C"
      value_template: "{{ value_json.temperature }}"

    - name: "SDI12 EC Sensor"
      state_topic: "sdi12/sensor"
      unit_of_measurement: "ms/cm"
      value_template: "{{ value_json.ec }}"
```

Arduino sketch: 

```
#include <WiFi.h>
#include <PubSubClient.h>
#include <esp32-sdi12.h>

// WiFi credentials
const char* ssid = "Wifi Name";
const char* password = "Wifi Password ";

// MQTT broker information
const char* mqtt_server = "192.168.50.196";
const int mqtt_port = 1883;
const char* mqtt_user = "mqtt";
const char* mqtt_password = "ttqm";
const char* mqtt_topic = "sdi12/teros-12";

#define SDI12_DATA_PIN 26 // Change based on the pin you are using
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

```

