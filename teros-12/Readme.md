
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

Arduino sketch: (Updated 03/007 with an attempt at callibrating the EC to the Teros 12 Solus Bluetooth dongle which I am using as benchmark).

Did a bit of really scientific testing using the previous arduino code. 

| Cup of 3.2 EC Athena   |                                                    | VWC   | EC   |
| ---------------------- | -------------------------------------------------- | ----- | ---- |
|                        | Teros 12 (using actual factory readings)           | 86    | 3.8  |
|                        | Teros China (using arduino code esp32)             | 92.86 | 3.08 |
|                        | Teros 12 ESP32 (using this arduino code and esp32) | 86    | 5.7  |
| Rockwool Cube          | Teros 12 (using actual factory readings)           | 70.42 | 3.81 |
|                        | Teros China (using arduino code esp32)             | 69.74 | 2.29 |
|                        | Teros 12 ESP32 (using this arduino code and esp32) | 71.76 | 4.21 |
| Rockwool Cube Side 2   | Teros 12 (using actual factory readings)           | 68.41 | 3.92 |
|                        | Teros China (using arduino code esp32)             | 64.64 | 1.96 |
|                        | Teros 12 ESP32 (using this arduino code and esp32) | 69.61 | 4.12 |
| Rockwool Cube half way | Teros 12 (using actual factory readings)           | 49.5  | 4.18 |
|                        | Teros China (using arduino code esp32)             | 51    | 1.14 |
|                        | Teros 12 ESP32 (using this arduino code and esp32) | 50.59 | 2.44 |

The new calibration below seems to get the EC of the Teros China and Teros 12 using ESP32 closer to the Teros 12 Solus Bluetooth app readingbut not perfect more work needs to be done. 

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

#define SDI12_DATA_PIN 2 // Change based on the pin you are using
#define DEVICE_ADDRESS uint8_t(0) // SDI-12 Address of device

ESP32_SDI12 sdi12(SDI12_DATA_PIN);
float values[10]; // Buffer to hold values from a measurement

WiFiClient espClient;
PubSubClient client(espClient);

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
    float vwc = (6.771e-10 * pow(raw, 3) - 5.105e-6 * pow(raw, 2) + 1.302e-2 * raw - 10.848);
    return constrain(vwc * 100.0, 0.0, 100.0);  // Convert to percentage and constrain between 0% and 100%
}

// EC calibration function
float calibrateEC(float rawEC) {
    return -0.1959 * rawEC + 4.4033;
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
        float raw_vwc = values[0];
        float vwc = calculateVWC(raw_vwc);
        float temperature = values[1]; // Temperature as is since it's assumed correct (in °C)
        
        // EC calculation with new calibration
        float ec_raw = values[2] / 1000.0;  // Convert µS/cm to dS/m
        float ec = calibrateEC(ec_raw);
        
        // Ensure EC is within the specified range and round to 3 decimal places
        ec = constrain(ec, 0.0, 23.0);
        ec = round(ec * 1000.0) / 1000.0;

        // Print values for debugging
        Serial.printf("Raw VWC: %f, VWC: %f%%, Temperature: %f°C, Raw EC: %f, Calibrated EC: %f dS/m\n", 
                      raw_vwc, vwc, temperature, ec_raw, ec);

        // Create JSON payload
        String payload = "{\"raw_vwc\": " + String(raw_vwc) + 
                         ", \"vwc\": " + String(vwc) + 
                         ", \"temperature\": " + String(temperature) + 
                         ", \"raw_ec\": " + String(ec_raw) + 
                         ", \"ec\": " + String(ec) + "}";
        client.publish(mqtt_topic, payload.c_str());
    }

    delay(15000); // Do this measurement every 15 seconds
}

```

