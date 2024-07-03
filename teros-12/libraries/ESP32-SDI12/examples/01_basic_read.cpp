#include <Arduino.h>
#include <esp32-sdi12.h>

#define SDI12_DATA_PIN 0

ESP32_SDI12 sdi12(SDI12_DATA_PIN);
float values[10];

void setup() {
    Serial.begin(115200);

    // Initialise SDI-12 pin definition
    sdi12.begin();
}

void loop() {
    // Measure on address 1
    // Float response will be inserted into values buffer
    ESP32_SDI12::Status res = sdi12.measure(1, values, sizeof(values));
    if(res != ESP32_SDI12::SDI12_OK){
        Serial.printf("Error: %d\n", res);
    }

    // Do something with the data here...

    delay(15000); // Do this measurement every 15 seconds
}
