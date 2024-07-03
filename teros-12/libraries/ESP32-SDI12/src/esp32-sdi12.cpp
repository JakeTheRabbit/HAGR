#include <Arduino.h>
#include "esp32-sdi12.h"

/**
 * @brief Constructor for ESP32_SDI12 object.
 *
 * Takes a single argument referencing a pin that is attached to the SDI-12
 * data line. This should be called before the setup() function under normal
 * operation.
 *
 * @param pin SDI-12 data pin (single pin all SDI-12 devices are connected to.
 */
ESP32_SDI12::ESP32_SDI12(int8_t pin){
    // Set the SDI12 pin definition to a variable for later use.
    ESP32_SDI12::sdi12_pin = pin;
}

/**
 * @brief Start SDI-12 service.
 *
 * @details This method initialises and instance of the ESPSoftwareSerial library with
 * the required settings for SDI-12 communications. This could be done in the
 * constructor but this gives further flexibility to the user.
 *
 * @see https://github.com/plerup/espsoftwareserial
 */
void ESP32_SDI12::begin() {
    // Pass baud rate (1200 as per the SDI-12 specsheet) and specified SDI-12
    // pin to software serial service.
    // 'SWSERIAL_7E1' specifies the SDI-12 bit frame format i.e.
    // 1 start bit, 7 data bits (7), 1 even parity bit (E) and 1 data bit (1).
    // TRUE required as the SDI-12 signal is normally low.
    // Valid GPIO pin will be checked by SoftwareSerial
    uart.begin(SDI12_BAUD, SWSERIAL_7E1, sdi12_pin, sdi12_pin, true);

    // If the pull-up is enabled during the command/response cycle, some
    // SDI-12 sensors do not work. We found the Meter Atmos41 failed in
    // this configuration.
    uart.enableRxGPIOPullUp(false);

    // As the SDI-12 communication operates in half-duplex mode (there is no
    // separate RX and TX lines) the interrupt on TX is turned off.
    uart.enableIntTx(false);

    // Placing the pin in a low state seems to help stability on startup.
    digitalWrite(sdi12_pin, LOW);
}

/**
 *
 * @brief Waits for a response from a SDI-12 device.
 *
 * Wait for a character to appear over UART before continuing. Timeout will
 * depend on a sensors response to a measure command. For example, if a
 * measure command is send, the preceding response should be ttt where ttt
 * represents the time in seconds until the device will be ready to send data.
 *
 * @see SDI-12 specification page 19.
 *
 * @param timeout the number of milliseconds to wait before timing out.
 * @return SDI12_OK if a character appears before the timeout,
 * otherwise SDI12_TIMEOUT.
 */
ESP32_SDI12::Status ESP32_SDI12::waitForResponse(uint32_t timeout) {

    uint32_t start = millis();
    while (millis() - start < timeout && !uart.available()) {
        delay(1);
    }

    #ifdef SDI12_SERIAL_DEBUG
        uint32_t end = millis();
        Serial.printf("Time elapsed: %u ms\n", end - start);
        if(!uart.available()){
            Serial.printf("No response was found before timeout\n");
        }
    #endif

    return uart.available() > 0 ? SDI12_OK : SDI12_TIMEOUT;
}

/**
 * @brief Passes a response into msg_buf.
 *
 * This method assumes data is available on the UART.
 *
 * @return The length of the message. If 0 there was no response.
 */
size_t ESP32_SDI12::readUntilCRLF() {
    // Reset the message buffer
    memset(msg_buf, 0, sizeof(msg_buf));

    // Read response from device
    uart.readBytesUntil('\n', msg_buf, MAX_RES_SIZE);

    #ifdef SDI12_SERIAL_DEBUG
        Serial.printf("Response length: %zu\n", strlen(msg_buf));
    #endif

    return strlen(msg_buf);
}

/**
 * @brief Main function to deal with SDI12 commands and responses.
 *
 * @details A sensor command is created based on the passed command request. Optionally,
 * the user can pass in a position or newAddress for commands that requires
 * these additional variables.
 *
 * @note SDI-12 structure is as follows.
 * Break (12 ms)
 *       │                          380 - 810 ms
 *       ▼      Command              Response
 *     ┌───┐  ┌─┐ ┌─┐ ┌─┐           ┌─┐ ┌─┐ ┌─┐
 *     │   │  │ │ │ │ │ │           │ │ │ │ │ │
 * ────┘   └──┘ └─┘ └─┘ └───────────┘ └─┘ └─┘ └─
 *           ▲               Max
 *           │          ◄───15 ms───►
 *       Marking (8.3 ms)
 *
 * @param address The sensor SDI-12 address.
 * @param cmd Command found in Commands enum.
 * @param position As a maximum of 75 bytes can be transmitted in a single
 * response, the position gives the ability to send multiple identical requests
 * while adjusting the position 0 - 9 to get all data.
 * @param newAddress If changing address this will be the sensors new address.
 * @return Status code (ESP32_SDI12::Status).
 */
ESP32_SDI12::Status ESP32_SDI12::querySensor(uint8_t address,
                                             Commands cmd,
                                             uint8_t position,
                                             uint8_t newAddress) {
    memset(sensor_cmd, 0, sizeof(sensor_cmd));

    bool response = false; // Is a response needed from this command?

    switch(cmd){
        case SDI12_Measure:
            response = true;
            snprintf(sensor_cmd, sizeof(sensor_cmd), "%dM!", address);
            break;
        case SDI12_CRC_Measure:
            response = true;
            snprintf(sensor_cmd, sizeof(sensor_cmd), "%dMC!", address);
            break;
        case SDI12_AD_Measure:
            response = true;
            snprintf(sensor_cmd, sizeof(sensor_cmd), "%dM%d!", address,
                     position);
            break;
        case SDI12_AD_CRC_Measure:
            response = true;
            snprintf(sensor_cmd, sizeof(sensor_cmd), "%dM%dC!", address,
                     position);
            break;
        case SDI12_Data:
            response = true;
            snprintf(sensor_cmd, sizeof(sensor_cmd), "%dD%d!", address,
                     position);
            break;
        case SDI12_Attention:
            snprintf(sensor_cmd, sizeof(sensor_cmd), "%d!", address);
            break;
        case SDI12_Information:
            response = true;
            snprintf(sensor_cmd, sizeof(sensor_cmd), "%dI!", address);
            break;
        case SDI12_CNC_Measure:
            response = true;
            snprintf(sensor_cmd, sizeof(sensor_cmd), "%dC!", address);
            break;
        case SDI12_CND_CRC_Measure:
            response = true;
            snprintf(sensor_cmd, sizeof(sensor_cmd), "%dCC!", address);
            break;
        case SDI12_Verify:
            response = true;
            snprintf(sensor_cmd, sizeof(sensor_cmd), "%dV!", address);
            break;
        case SDI12_Change_Address:
            snprintf(sensor_cmd, sizeof(sensor_cmd), "%dA%d!", address,
                     newAddress);
            break;
    }

    #ifdef SDI12_SERIAL_DEBUG
        Serial.printf("Issuing command: %s\n", sensor_cmd);
    #endif // SDI12_SERIAL_DEBUG

    // Reset the message buffer
    memset(msg_buf, '\0', sizeof(msg_buf));

    uart.enableRx(false); // Disable interrupts on RX

    pinMode(sdi12_pin, OUTPUT); // Set pin to OUTPUT mode before break and marking

    // Break (at least 12 ms) put signal HIGH
    digitalWrite(sdi12_pin, HIGH);
    delay(13);

    // Marking (at least 8.3 ms) pull signal LOW
    digitalWrite(sdi12_pin, LOW);
    delay(9);

    // Send command after break and marking to sensor
    uart.write(sensor_cmd);

    // Receive message from sensor
    uart.enableTx(false); // Enables RX mode

    Status res = waitForResponse();
    if (res != SDI12_OK) {
        return res;
    }

    if (response) {
        // readUntilCRLF leaves the string read from the UART in msg_buf with
        // trailing whitespace removed.
        readUntilCRLF();
    }

    digitalWrite(sdi12_pin, LOW);

    return SDI12_OK;
}


/**
 * @brief Executes the SDI-12 attention command to see if a sensor is available
 * on a particular address.
 *
 * @warning Not all sensors respond to this command. Use information command
 * if unsure.
 *
 * @param address SDI-12 sensor address
 * @return Status code (ESP32_SDI12::Status).
 */
ESP32_SDI12::Status ESP32_SDI12::ackActive(uint8_t address) {

    if(validAddress(address) != SDI12_OK){
        return SDI12_INVALID_ADDR;
    }

    return querySensor(address, SDI12_Attention);
}

/**
 * @brief Get SDI-12 sensor information.

 * Pass in a reference to a Sensor struct to be populated (when successful).
 *
 * @param address SDI-12 sensor address.
 * @param sensor SDI-12 Sensor struct to be populated.
 * @return Status code (ESP32_SDI12::Status).
 */
ESP32_SDI12::Status ESP32_SDI12::sensorInfo(uint8_t address,
                                            Sensor* sensor) {

    if(validAddress(address) != SDI12_OK){
        return SDI12_INVALID_ADDR;
    }

    ESP32_SDI12::Status res = querySensor(address, SDI12_Information);
    if(res != SDI12_OK) {
        return res;
    }

    // Parse response held in msg_buf into a SDI12 sensor object
    memcpy(sensor, msg_buf, strlen(msg_buf));

    #ifdef SDI12_SERIAL_DEBUG
        sensor_debug(sensor);
    #endif // SDI12_SERIAL_DEBUG

    return SDI12_OK;
}

/**
 * @breif Get SDI-12 sensor information.
 *
 * Used when the ackActive() command is not supported by the sensor. This
 * method will return SDI12_OK if a device is found but will not populate
 * a Sensor struct.
 *
 * @param address SDI12 sensor address.
 * @return Status code (ESP32_SDI12::Status).
 */
ESP32_SDI12::Status ESP32_SDI12::sensorInfo(uint8_t address) {

    if(validAddress(address) != SDI12_OK){
        return SDI12_INVALID_ADDR;
    }

    ESP32_SDI12::Status res = querySensor(address, SDI12_Information);
    if(res != SDI12_OK) {
        return res;
    }

    return SDI12_OK;
}

/**
 * @brief Checks if the supplied address is a valid SDI-12 address.
 *
 * @param address SDI-12 sensor address.
 * @return Status code (ESP32_SDI12::Status).
 */
ESP32_SDI12::Status ESP32_SDI12::validAddress(uint8_t address) {
    return (address <= MAX_N_DEVICES) ? SDI12_OK : SDI12_INVALID_ADDR;
}

/**
 * @brief Obtains an overview of sensors on the SDI-12 bus.
 *
 * Populates a Sensors struct with sensors as they are found on the SDI-12 bus.
 *
 * @param sensors Sensors struct to be populated.
 * @return Status code (ESP32_SDI12::Status).
 */
ESP32_SDI12::Status ESP32_SDI12::sensorsOnBus(Sensors* sensors) {

    sensors->count = 0;
    for (uint8_t address = 0; address <= 9; address++) {
        if (querySensor(address, SDI12_Information) == SDI12_OK) {
            // Assign sensor to provided struct
            memcpy(&sensors->sensor[sensors->count], msg_buf, strlen(msg_buf));

            // Doing a memcpy above results in the address becoming its decimal
            // representation e.g. 0 becomes 48. To solve this, the sensors
            // address is reassigned with its actual address.
            sensors->sensor[sensors->count].address = address;

            #ifdef SDI12_SERIAL_DEBUG
                sensor_debug(&sensors->sensor[sensors->count]);
            #endif // SDI12_SERIAL_DEBUG

            sensors->count++;

        } else {
            #ifdef SDI12_SERIAL_DEBUG
                Serial.printf("No sensor found on address: %d\n", address);
            #endif // SDI12_SERIAL_DEBUG
        }
    }

    if (sensors->count == 0) {
        return SDI12_SENSOR_NOT_FOUND;
    }

    #ifdef SDI12_SERIAL_DEBUG
        Serial.printf("=== Sensor scan complete ===\n");
        Serial.printf("\t%d SDI12 sensor(s) found.\n", sensors->count);
    #endif // SDI12_SERIAL_DEBUG

    return SDI12_OK;
}

/**
 * @brief Change a devices SDI-12 address.
 *
 * @param address Sensors current SDI-12 address.
 * @param newAddress Proposed SDI-12 sensor address.
 * @return Status code (ESP32_SDI12::Status).
 */
ESP32_SDI12::Status ESP32_SDI12::changeAddress(uint8_t address,
                                               uint8_t newAddress) {
    #ifdef SDI12_SERIAL_DEBUG
        Serial.printf("Request sensor change address from: %d to: %d\n",
                      address, newAddress);
    #endif // SDI12_SERIAL_DEBUG

    // Check if addresses are valid
    if(validAddress(address) != SDI12_OK &&
       validAddress(newAddress) != SDI12_OK){
        return SDI12_INVALID_ADDR;
    }

    // Check if devices exist on each address (should on the first, shouldn't
    // on the new address). Using the information command here as not all
    // sensors support the verify command.
    Status res = sensorInfo(address);
    if(res != SDI12_OK){
        #ifdef SDI12_SERIAL_DEBUG
            Serial.printf("Error, no sensor found on address %d\n", address);
        #endif // SDI12_SERIAL_DEBUG
        return SDI12_SENSOR_NOT_FOUND;
    }

    res = sensorInfo(newAddress);
    if(res == SDI12_OK){
        #ifdef SDI12_SERIAL_DEBUG
            Serial.printf("Error, address %d already inuse\n", newAddress);
        #endif // SDI12_SERIAL_DEBUG
        return SDI12_ADDR_IN_USE;
    }

    // Change address from address to new address
    res = querySensor(address, SDI12_Change_Address, 0, newAddress);

    #ifdef SDI12_SERIAL_DEBUG
        if (res == SDI12_OK) {
            Serial.printf("Address changed from %d to %d\n", address, newAddress);
        }
    #endif // SDI12_SERIAL_DEBUG

    return res;
}

/**
 * @brief Request a SDI-12 sensor to take a measurement.
 *
 * If successful the Measure struct will contain all variables needed
 * to submit a subsequent data command (as required by the SDI-12 spec).
 *
 * @note This method does not return sensor data, rather information regarding
 * how long it will take for the sensor to have data ready to send and how many
 * values can be expected.
 *
 * @param address Sensor SDI-12 address.
 * @param measure Measure struct to be populated (if successful)
 * @return Status code (ESP32_SDI12::Status).
 */
ESP32_SDI12::Status ESP32_SDI12::requestMeasure(uint8_t address,
                                                Measure* measure) {
    Status res = querySensor(address, SDI12_Measure, 0, 0);

    #ifdef SDI12_SERIAL_DEBUG
        if(res != SDI12_OK){
            Serial.printf("Error requesting sensor measurement (Error = %d)\n", res);
            return res;
        } else {
            Serial.printf("Successfully queued measurement on address: %d\n", address);
        }
    #endif // SDI12_SERIAL_DEBUG

    // Device address that requestMeasure command was issued on
    measure->address = msg_buf[0] - '0';

    // Delay time before data is ready in seconds
    char delay_time_buf[4];
    for(uint8_t i = 1; i < 4; i++){
       delay_time_buf[i-1] = msg_buf[i];
       delay_time_buf[i] = 0;
    }
    measure->delay_time = strtol(delay_time_buf, nullptr, 10);

    // Expected number of values in response
    measure->n_values = msg_buf[4] - '0';

    // A sensor *should* send a service request string when it has asked for a delay of > 0
    // seconds. We have at least one sensor that does not always do this so don't assume
    // it's coming, but read and discard it if it does.
    if (measure->delay_time > 0) {
        waitForResponse(measure->delay_time * 1000);
        if (uart.available() > 0) {
            // This should be the service request, because no data commands
            // have been issued yet.
            readUntilCRLF();
        }
    }

    return SDI12_OK;
}

/**
 * Request data from SDI-12 sensor (after measure command).
 *
 * @breif Used to request data from a sensor after a sensor measure command has
 * been executed.
 *
 * @note Multiples of this request may be needed to get all values from a sensor
 * this can be handled by incrementing the position variable from 0 through to
 * 9 with each subsequent request.
 *
 * @param address Sensor SDI-12 address.
 * @param position Position in data command interactions.
 * @return Status code (ESP32_SDI12::Status).
 */
ESP32_SDI12::Status ESP32_SDI12::requestData(uint8_t address,
                                             uint8_t position) {
    Status res = querySensor(address, SDI12_Data, position, 0);
    #ifdef SDI12_SERIAL_DEBUG
        if(res != SDI12_OK){
            Serial.printf("Error requesting sensor data (Error = %d)\n", res);
        } else {
            Serial.printf("Successfully obtained measurement "
                          "data on address: %d\n", address);
        }
    #endif // SDI12_SERIAL_DEBUG

    return res;
}

/**
 * Ask SDI-12 sensor to take a measurement and return all values.
 *
 * @breif Combines the requestMeasure() and requestData() commands to ask a
 * sensor to take any measurements and wait for a response then return all
 * these data as floats to the user. This will handle multiple data commands
 * and only return when all values have been received.
 *
 * @code
 * static float values[10];
 * static uint8_t n_received;
 * ESP32_SDI12::Status res = sdi12.measure(address, values,
 *                                          sizeof(values), &n_received);
 *
 * @param address Sensor SDI-12 address.
 * @param values User supplied buffer to hold returned measurement values.
 * @param max_values Number of values in user supplied buffer.
 * @param num_returned_values The number of values read back from the sensor.
 * @return Status code (ESP32_SDI12::Status).
 */
ESP32_SDI12::Status ESP32_SDI12::measure(uint8_t address,
                                         float* values,
                                         size_t max_values,
                                         uint8_t* num_returned_values) {

    Measure measure = {};

    // Request a new measurement. This call parses the response to the measure
    // command, delays as necessary and reads the attention sequence. Upon
    // return the data commands can be issued immediately.
    Status res = requestMeasure(address, &measure);
    if (res != SDI12_OK) {
        return res;
    }

    // Tell the caller they did not provide enough space for the measurements.
    if (measure.n_values >= max_values) {
        return SDI12_BUF_OVERFLOW;
    }

    uint8_t parsed_values = 0; // Number of values successfully parsed
    // Position in the data command request (multiple calls may be needed to
    // get all values from a sensor).
    uint8_t position = 0;
    while (parsed_values < measure.n_values) {
        // Request data as it should be ready to be read now
        res = requestData(address, position);
        if (res != SDI12_OK) {
            return res;
        }

        char* msg_ptr;
        // Extracts the device address and stores a ptr to the rest of the
        // message buffer for use below (to extract values only)
        strtof(msg_buf, &msg_ptr);
        char* next_msg_ptr;
        float value;
        // Extract the values from the message buffer and put into user
        // supplied buffer
        for (size_t i = 0; i < max_values; i++) {
            value = strtof(msg_ptr, &next_msg_ptr);
            if(msg_ptr == next_msg_ptr){
                break;
            }
            #ifdef SDI12_SERIAL_DEBUG
                Serial.printf("Value: %f\n", value);
            #endif // SDI12_SERIAL_DEBUG
            values[parsed_values++] = value;
            msg_ptr = next_msg_ptr;
        }

        // Increment the position in the data command to get more measurements
        // until all values hav been received
        position++;
    }

    // Assign the number for returned values to the user provided variable
    // Skip of num_returned_values is a nullptr
    if (num_returned_values) {
        *num_returned_values = measure.n_values;
    }

    return res;
}

/* DEBUG methods below */
#ifdef SDI12_SERIAL_DEBUG
/**
 * Helper function to display sensor information as Serial output.
 *
 * @param sensor Sensor struct to display.
 */
void ESP32_SDI12::sensor_debug(Sensor* sensor){

    Serial.printf("=== SDI12 sensor ===\n");
    Serial.printf("\tAddress: %c\n", (char)sensor->address);
    Serial.printf("\tSDI12 Version: v%c.%c\n",(char)sensor->sdi_version_major,
              (char)sensor->sdi_version_minor);

    Serial.printf("\tVendor: ");
    for(unsigned char i : sensor->vendor){
        char c = (char)i;
        if(c == '\0') break;
        Serial.printf("%c", c);
    }

    Serial.printf("\n\tModel: ");
    for(unsigned char i : sensor->model){
        char c = (char)i;
        if(c == '\0') break;
        Serial.printf("%c", c);
    }

    Serial.printf("\n\tSensor Version: ");
    for(unsigned char i : sensor->sensor_version){
        char c = (char)i;
        if(c == '\0') break;
        Serial.printf("%c", c);
    }

    Serial.printf("\n\tAux Info: ");
    for(unsigned char i : sensor->aux_information){
        char c = (char)i;
        if(c == '\0') break;
        Serial.printf("%c", c);
    }

    Serial.printf("\nRaw sensor response: %s\n", msg_buf);
}
#endif // SDI12_SERIAL_DEBUG
