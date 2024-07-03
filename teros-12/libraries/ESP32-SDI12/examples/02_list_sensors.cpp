#include <Arduino.h>
#include <esp32-sdi12.h>

#define SDI12_DATA_PIN 0

ESP32_SDI12 sdi12(SDI12_DATA_PIN);
ESP32_SDI12::Sensors sensors;

void setup() {
    Serial.begin(115200);

    // Initialise SDI-12 pin definition
    sdi12.begin();
}

void loop() {
    // Measure on address 1
    // Float response will be inserted into values buffer
    ESP32_SDI12::Status res = sdi12.sensorsOnBus(&sensors);

    if(res == ESP32_SDI12::SDI12_OK){
        Serial.printf("Success.\n");
        for(uint8_t i = 0; i < sensors.count; i++){
            Serial.printf("Address: %d, Vendor: %s, Model: %s.\n",
                          sensors.sensor[i].address,
                          sensors.sensor[i].vendor,
                          sensors.sensor[i].model);
        }
    } else {
        Serial.printf("Error: %d\n", res);
    }

    // Do something with the sensors here...

    delay(15000);
}
