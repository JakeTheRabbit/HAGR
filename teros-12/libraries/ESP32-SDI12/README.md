# ESP32-SDI12

Utilises the [ESPSoftwareSerial](https://github.com/plerup/espsoftwareserial) library to provide interfacing with SDI-12 sensors via a UART connection in half-duplex mode (single pin required).

# Basic operation

```c++
#include <esp32-sdi12.h>

#define SDI12_DATA_PIN D0 // Change based on the pin you are using
#define DEVICE_ADDRESS 1 // SDI-12 Address of device

// Create a SDI12 object, passing in your chosen pin
ESP32_SDI12 sdi12(SDI12_DATA_PIN);

// Buffer to hold values from a measurement
float values[10];

void setup() {
    Serial.begin(115200);

    // Initialise SDI-12 pin definition
    sdi12.begin();
}

void loop() {
    // Measure (will populate values array with data)
    ESP32_SDI12::Status res = sdi12.measure(DEVICE_ADDRESS, values, sizeof(values));
    
    // Error handling
    if(res != ESP32_SDI12::SDI12_OK){
        // Some error occured, handle it here
        Serial.printf("Error: %d\n", res);
    }
    
    // Do what you want with values here

    delay(15000); // Do this measurement every 15 seconds
}
```

# Licence
This project is under the GNU LESSER GENERAL PUBLIC LICENSE as found in the LICENCE file.
