# M5Stack Dial Tank Monitor

![Tank Monitor Demo](tank-monitor-demo.jpg)

A sophisticated water tank level monitoring system using an M5Stack Dial and ultrasonic distance sensor. The sensor is mounted on the inside of the tank lid to provide accurate water level readings displayed as both distance and percentage values.

## Features

- **Real-time Water Level Monitoring**: Continuous distance measurement converted to tank percentage
- **Color-coded Display**: Visual indication of tank status (Red = Empty, Orange = Low, Yellow = Medium, Green = Full)
- **Interactive Calibration**: Easy setup for empty and full tank levels using the rotary encoder
- **Smart Display Management**: Auto-timeout with manual override option
- **Home Assistant Integration**: Full integration with sensors, switches, and controls
- **Audio Feedback**: Menu navigation sounds and confirmation beeps
- **Web Interface**: Built-in web server for remote monitoring
- **Robust Design**: Power management for reliable M5Stack Dial operation

## Hardware Required

### Main Components
- **M5Stack Dial** (ESP32-S3 based device with rotary encoder and round display)
- **Ultrasonic I2C Distance Sensor** (compatible with address 0x57)
- **Water tank** with accessible lid for sensor mounting

### Connections
- Ultrasonic sensor connects to **Port A** (GPIO13 SDA, GPIO15 SCL)
- No additional wiring required - uses M5Stack Dial's built-in components

## Installation

### 1. Hardware Setup

1. **Mount the Ultrasonic Sensor**:
   - Install the sensor on the **inside of the tank lid**
   - Position it to point straight down toward the water surface
   - Ensure the sensor face is clean and unobstructed
   - Secure with appropriate mounting hardware

2. **Connect the Sensor**:
   - Connect the ultrasonic sensor to the M5Stack Dial's Port A
   - Use the Grove connector or appropriate I2C wiring

### 2. ESPHome Configuration

1. **Prepare secrets.yaml**:
   ```yaml
   wifi_ssid: "YourWiFiNetwork"
   wifi_password: "YourWiFiPassword"
   api_encryption_key: "your-32-character-api-key-here"
   ota_password: "your-ota-password-here"
   ```

2. **Compile and Upload**:
   ```bash
   esphome compile m5stack-dial-tank-monitor.yaml
   esphome upload m5stack-dial-tank-monitor.yaml
   ```

3. **Customize Network Settings** (if needed):
   - Update the static IP address in the configuration
   - Modify WiFi fallback hotspot credentials

## Calibration Process

### Important: Calibrate with Tank Empty and Full

The system needs to know the distance measurements for both empty and full tank states.

1. **Empty Tank Calibration**:
   - Ensure your tank is completely empty
   - Press the center button to enter settings menu
   - Navigate to "Empty Level" using the rotary encoder
   - Press the button to start calibration mode
   - Press the button again to save the current distance reading

2. **Full Tank Calibration**:
   - Fill your tank completely
   - Navigate to "Full Level" in the settings menu
   - Follow the same process to calibrate the full level

## Usage

### Main Display

The home screen shows:
- **Tank Level Percentage** (large, color-coded)
- **Current Distance Reading** (in mm)
- **Calibration Values** (Empty/Full distances)
- **Visual Tank Indicator** (filled based on percentage)
- **Screen Timeout Counter** (or "ON" if always-on mode enabled)

### Navigation
- **Rotary Encoder**: Navigate through settings menu
- **Center Button Press**: Enter settings or confirm selections
- **Settings Menu Options**:
  - Empty Level Calibration
  - Full Level Calibration
  - Screen Timeout Toggle
  - Exit Settings

### Display Color Coding

| Tank Level | Color | Description |
|------------|-------|-------------|
| 0-19% | Red | Empty - Refill needed |
| 20-39% | Orange | Low - Monitor closely |
| 40-69% | Yellow | Medium - Normal usage |
| 70-100% | Green | Full - Optimal level |

## Home Assistant Integration

### Automatic Discovery
Once connected to your network, the device will automatically appear in Home Assistant with these entities:

#### Sensors
- `sensor.tank_level` - Tank level percentage
- `sensor.tank_distance` - Raw distance measurement
- `sensor.tank_monitor_uptime` - Device uptime
- `sensor.wifi_signal` - WiFi signal strength

#### Controls
- `switch.screen_always_on` - Disable screen timeout
- `switch.menu_sounds` - Enable/disable audio feedback
- `switch.display` - Turn display on/off
- `switch.restart_tank_monitor` - Remote restart

#### Configuration
- `number.tank_empty_distance` - Adjust empty level distance
- `number.tank_full_distance` - Adjust full level distance

### Example Automation

```yaml
# Send notification when tank is low
automation:
  - alias: "Tank Low Warning"
    trigger:
      - platform: numeric_state
        entity_id: sensor.tank_level
        below: 20
    action:
      - service: notify.mobile_app
        data:
          message: "Water tank is low ({{ states('sensor.tank_level') }}%). Consider refilling."
          title: "Tank Monitor Alert"
```

## Configuration Options

### Key Parameters to Adjust

```yaml
# Update intervals
update_interval: 2s  # Distance sensor reading frequency
update_interval: 5s  # Tank level calculation frequency

# Display timeout (in seconds)
display_timeout_seconds: 300  # 5 minutes default

# Network settings
static_ip: 192.168.73.158  # Change to match your network
gateway: 192.168.73.1
subnet: 255.255.255.0
```

### Sensor Address
The ultrasonic sensor uses I2C address `0x57`. If your sensor uses a different address, update:
```yaml
address: 0x57  # Change if your sensor uses different address
```

## Troubleshooting

### Common Issues

**Display not turning on:**
- Check the hold_power output (GPIO46) is properly configured
- Verify power connections

**Sensor not reading:**
- Check I2C connections to Port A (GPIO13/GPIO15)
- Verify sensor is powered and at correct address
- Use I2C scan to detect sensor

**Inaccurate readings:**
- Recalibrate empty and full levels
- Check sensor mounting (should point straight down)
- Ensure sensor face is clean
- Verify tank is actually empty/full during calibration

**WiFi connection issues:**
- Check credentials in secrets.yaml
- Verify network settings match your setup
- Use fallback hotspot for initial configuration

### Logs and Debugging

Enable debug logging by setting:
```yaml
logger:
  level: DEBUG
```

Monitor logs for:
- Distance sensor readings
- Calibration values
- Menu navigation events
- WiFi connectivity

## Technical Specifications

- **Microcontroller**: ESP32-S3 (M5Stack Dial)
- **Display**: 1.28" Round LCD (240x240)
- **Sensor Interface**: I2C (100kHz)
- **Power**: USB-C or battery (with power management)
- **WiFi**: 802.11 b/g/n
- **Measurement Range**: Depends on ultrasonic sensor (typically 30-4000mm)
- **Accuracy**: ±1mm (sensor dependent)
- **Update Rate**: 2-5 seconds

## Dependencies

- [ESPHome](https://esphome.io/)
- [M5Stack-ESPHome Components](https://github.com/chill-Division/M5Stack-ESPHome/) (sonic_i2c)
- Arial font file (arial.ttf)

## License

Open source hardware and software project. Feel free to modify and adapt for your needs.

## Contributing

Contributions welcome! Please submit issues and pull requests on the GitHub repository.

---

**Note**: This project requires an ultrasonic distance sensor mounted inside your tank lid. Ensure proper waterproofing and safe installation practices when working with water storage systems.