#include <Arduino.h>
#include <esp32-sdi12.h>

#define SDI12_DATA_PIN 13

static ESP32_SDI12 sdi12(SDI12_DATA_PIN);
static ESP32_SDI12::Sensors sensors;
static float values[10];

static uint8_t addr;
static uint8_t n_returned_values;

void setup() {
    // Setup serial for debugging
    Serial.begin(115200);
    while(!Serial){
        delay(1);
    }

    // Initialise SDI-12 pin definition
    sdi12.begin();

    // Get a list of sensors on the bus on startup
    ESP32_SDI12::Status res = sdi12.sensorsOnBus(&sensors);

    if(res != ESP32_SDI12::SDI12_OK){
        // No sensors found, stop execution
        Serial.printf("No SDI-12 sensors found. Stopping.\n");
        while(true){ delay(1); }
    } else {
        // Devices found
        Serial.printf("%d SDI-12 sensors found.\n", sensors.count);
    }

}

void loop() {

    // Loop through sensors that were found on the bus
    for(uint8_t i = 0; i < sensors.count; i++){
        // Get the sensors address
        addr = sensors.sensor[i].address;

        // Take a measurement
        ESP32_SDI12::Status res = sdi12.measure(addr, values,
                                                sizeof(values),
                                                &n_returned_values);

        // Handle response status
        if(res  == ESP32_SDI12::SDI12_OK){
            // Measure was successful, print values
            Serial.printf("Measure complete.\n");
            for(uint8_t v = 0; v < n_returned_values; v++){
                Serial.printf("Value %d: %f\n", v + 1, values[v]);
            }
        } else {
            // Measure was unsuccessful check res against Status enum
            Serial.printf("Error: %d\n", res);
        }
    }

    // Wait for next measurement
    delay(20000);
}
