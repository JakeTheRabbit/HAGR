#ifndef ESP32_SDI12_H
#define ESP32_SDI12_H

#include "SoftwareSerial.h"

// Optional define to display SDI-12 serial debug output
//#define SDI12_SERIAL_DEBUG

class ESP32_SDI12 {
private:
    // Software Serial instance (initialised in `begin()`)
    SoftwareSerial uart;

    // Maximum number of SDI-12 devices on bus
    static constexpr int8_t MAX_N_DEVICES       =       9;

    /* Information command structure begin */
    // Length of vendor
    static constexpr int8_t LEN_VENDOR          =       8;
    // Length of sensor model identifier
    static constexpr int8_t LEN_MODEL           =       6;
    // Length of sensor version
    static constexpr int8_t LEN_SENSOR_VERSION  =       3;
    // Length of additional sensor information
    static constexpr int8_t LEN_INFO            =       13;
    /* Information command structure end */

    // Baud rate of SDI-12 TX/RX
    static constexpr int16_t SDI12_BAUD         =       1200;
    // Max size of any SDI-12 command (in bytes)
    static constexpr int8_t MAX_CMD_SIZE        =       8;
    // Max size of response (in bytes)
    static constexpr int8_t MAX_RES_SIZE        =       75;

    // Message buffer for the command
    char sensor_cmd[MAX_CMD_SIZE+1] = {0};
    // Message buffer for response
    char msg_buf[MAX_RES_SIZE+1] = {0};

    // List of available SDI-12 commands
    enum Commands {
        // aM! (Measure)
        SDI12_Measure           =       0x00,
        // aMC! (CRC Measure)
        SDI12_CRC_Measure       =       0x01,
        // aMd! (Additional Measure)
        SDI12_AD_Measure        =       0x02,
        // aMCd! (Additional CRC Measure)
        SDI12_AD_CRC_Measure    =       0x03,
        // aDp! (Get data)
        SDI12_Data              =       0x04,
        // a! (Attention)
        SDI12_Attention         =       0x05,
        // aI! (Information)
        SDI12_Information       =       0x06,
        // aC! (Concurrent Measure)
        SDI12_CNC_Measure       =       0x07,
        // aCC! (Concurrent CRC Measure)
        SDI12_CND_CRC_Measure   =       0x08,
        // aV! (Verify)
        SDI12_Verify            =       0x09,
        // aAb! (Change Address)
        SDI12_Change_Address    =       0x0A
    };

    // SDI-12 measure response structure
    struct Measure {
        uint8_t address;
        uint16_t delay_time;
        uint8_t n_values;
    };

public:
    // Status information for SDI-12 requests and data parsing in the
    // ESP32-SDI12 library
    enum Status {
        // All ok, no errors detected
        SDI12_OK                =       0x00,
        // A generic error was detected
        SDI12_ERR               =       0x01,
        // Sensor not found when querying the bus
        SDI12_SENSOR_NOT_FOUND  =       0x02,
        // Invalid SDI-12 address was supplied
        SDI12_INVALID_ADDR      =       0x03,
        // SDI-12 address already in-use by another sensor
        SDI12_ADDR_IN_USE       =       0x04,
        // SDI-12 response timed out
        SDI12_TIMEOUT           =       0x05,
        // Supplied buffer was too small to hold the returned data
        SDI12_BUF_OVERFLOW      =       0x06
    };

    // SDI-12 sensor information obtained from a `SDI12_Information`
    // request
    struct Sensor {
        // Sensor address on the bus (0 - 9)
        uint8_t address;
        // SDI-12 major version identifier
        uint8_t sdi_version_major;
        // SDI-12 minor version identifier
        uint8_t sdi_version_minor;
        // SDI-12 vendor identifier
        uint8_t vendor[LEN_VENDOR];
        // SDI-12 sensor model identifier
        uint8_t model[LEN_MODEL];
        // SDI-12 sensor version information
        uint8_t sensor_version[LEN_SENSOR_VERSION];
        // Any additional information from sensor
        uint8_t aux_information[LEN_INFO];
    };

    // Structure to hold up to 10 sensors (and all their information)
    struct Sensors {
        // Number of sensors detected on bus
        uint8_t count;
        // Array of sensor information
        Sensor sensor[MAX_N_DEVICES];
    };

    explicit ESP32_SDI12(int8_t pin);
    ~ESP32_SDI12() = default;

    void begin();
    Status ackActive(uint8_t address);
    Status sensorInfo(uint8_t address, Sensor* sensor);
    Status sensorInfo(uint8_t address);
    Status sensorsOnBus(Sensors* sensors);
    Status changeAddress(uint8_t address,
                         uint8_t newAddress);
    Status measure(uint8_t address, float* values, size_t max_values,
                   uint8_t* num_returned_values = nullptr);

    // Default SDI-12 pin (should error if uninitialized as
    // `SDI12_INVALID_ADDR`)
    int8_t sdi12_pin = -1;

private:
    Status waitForResponse(uint32_t timeout = 1000);
    Status querySensor(uint8_t address, Commands cmd, uint8_t position = 0,
                       uint8_t newAddress = 0);
    static Status validAddress(uint8_t address);
    Status requestMeasure(uint8_t address, Measure* measure);
    Status requestData(uint8_t address, uint8_t position);

    size_t readUntilCRLF();

#ifdef SDI12_SERIAL_DEBUG
    // Debug message that outputs sensor information over a serial connection
    void sensor_debug(Sensor* sensor);
#endif // SDI12_SERIAL_DEBUG

};

#endif //ESP32_SDI12_H
