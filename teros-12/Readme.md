# TEROS-12 Soil Moisture Sensor with ESP32 and MQTT

This project integrates TEROS-12 soil moisture sensors (and compatible alternatives) with ESP32 microcontrollers, sending data to Home Assistant via MQTT.

## Table of Contents
1. [Overview](#overview)
2. [Disclaimer](#disclaimer)
3. [Hardware](#hardware)
4. [Setup](#setup)
5. [Wiring](#wiring)
6. [TEROS-12 Compatible Sensor: BGT-SEC(Z2)](#teros-12-compatible-sensor-bgt-secz2)
7. [Home Assistant Configuration](#home-assistant-configuration)
8. [Sensor Comparison](#sensor-comparison)
9. [Web Interface](#web-interface)
10. [References](#references)

## Overview

This project connects TEROS-12 compatible soil moisture sensors to ESP32 microcontrollers and sends the data to Home Assistant using MQTT. It supports both the original TEROS-12 and a Chinese alternative (BGT-SEC(Z2)).
Originally inspired by https://github.com/kromadg/soil-sensor connecting the THC-S to Home Assistant. 

## Disclaimer

- This project was created during late-night coding sessions and may contain errors.
- Do not trust this implementation blindly; perform proper testing before relying on it.
- This is a work in progress and has not been fully tested yet.
- This guide assumes basic knowledge of Arduino IDE and may skip some steps.
- The project was developed using AI assistance (ChatGPT/Claude), so I don't have a full understanding and may have made blatant errors.

## Hardware

- ESP32 (tested with M5 Atom, M5 Atom S3, M5 PoEESP32, and M5 Dial)
- TEROS-12 soil moisture sensor or BGT-SEC(Z2) compatible sensor
- M5 Stack grove cable

## Setup

1. Download and install Arduino IDE
2. Download and install the [ESP32-SDI12 library](https://github.com/HarveyBates/ESP32-SDI12)
3. Get an M5 Stack grove cable and connect the cables (see [Wiring](#wiring) section)
4. Connect the Teros China 12 to the ESP32 and plug into your computer
5. Copy the .ino file into the Arduino IDE and flash the ESP32. Update all the variables at the top of the .ino file
6. Download the .h and .cpp files as well
7. For any errors, use ChatGPT or Claude for troubleshooting

## Wiring

### TEROS-12 Solus

![TEROS-12 Solus Wiring](https://github.com/JakeTheRabbit/HAGR/assets/123831499/6e8107e2-4be7-4fe8-b744-577949a88612)

![TEROS-12 Manual 3.5mm pin labels](https://github.com/JakeTheRabbit/HAGR/assets/123831499/767efecf-1174-4a88-aeeb-df83142dacce)

### BGT-SEC(Z2) (Chinese TEROS-12 compatible)

| Sensor Wire | Grove Cable |
|-------------|-------------|
| Red         | Yellow (Data) |
| White       | Red (Power) |
| Ground wire - needs extra wrapping to isolate | Black (Ground) |
| N/A         | White (Cut off and isolate) |

![BGT-SEC(Z2) Wiring](https://github.com/JakeTheRabbit/HAGR/assets/123831499/e233d3d0-7c3c-494e-95d3-3f13e804ed6c)

![Finished Connection](https://github.com/JakeTheRabbit/HAGR/assets/123831499/47168f2c-3daa-4163-83a8-019859e2bcde)

## TEROS-12 Compatible Sensor: BGT-SEC(Z2)

Our Chinese friends have produced a formidable alternative called the BGT-SEC(Z2). For this README, we'll refer to this sensor as Teros China and the original as Teros USA.

![BGT-SEC(Z2) Sensor and Manual](https://github.com/JakeTheRabbit/HAGR/assets/123831499/e1ecafbd-6714-4711-8370-85854ece9f82)

![Wiring Diagram](https://github.com/JakeTheRabbit/HAGR/assets/123831499/ca8fb432-2bd9-4ed3-9a15-d025028a4ff1)

![Technical Specs](https://github.com/JakeTheRabbit/HAGR/assets/123831499/846fea80-770f-48f2-a648-5776f9c4c94a)

![VWC Calibration Info](https://github.com/JakeTheRabbit/HAGR/assets/123831499/0df29d1f-5ede-48f9-87fc-102fb1d49f6a)

Where to buy: [Alibaba Link](https://www.alibaba.com/product-detail/China-low-price-CE-IP68-SID12_1600643601689.html) (choose the SDI-12 version, 5m cable recommended)


# New config 2025 for ESPHOME. This is all you need to connect to Home Assistant through ESP Home. This version also works with M5 Stack ESPOE Devices. 
Credits [Chill Division](https://github.com/Chill-Division/M5Stack-ESPHome/blob/main/VH3.96%20-%204Pin%20Transfer%20Module%20Unit.md)

Yaml for M5Stack Atom Lite: 

```
esphome:
  name: f1-row1-front-sdi12

esp32:
  board: m5stack-atom
#  board: m5stack-core-esp32 # (EsPoE Version)
  framework:
    type: esp-idf

# Enable logging
logger:

# Enable Home Assistant API
api:
  encryption:
    key: ""

ota:
  - platform: esphome
    password: ""

#ethernet:
#  type: IP101
#  mdc_pin: GPIO23
#  mdio_pin: GPIO18
#  clk_mode: GPIO0_IN
#  phy_addr: 1
#  power_pin: GPIO5
#  #use_address: 192.168.122.82

wifi:
  ssid: "Your_WiFi_SSID"  # Replace with your WiFi SSID
  password: "Your_WiFi_Password"  # Replace with your WiFi password
  
  # Enable fallback hotspot in case of connection failure
  ap:
    ssid: "F1-Row1-Front-Sdi12"
    password: "12345678"

web_server:
  port: 80
  local: true
  version: 2
  include_internal: true

external_components:
  - source: github://ssieb/esphome@uarthalf
    components: [ uart ]
    refresh: 1min
  - source: github://ssieb/esphome_components@sdi12
    components: [ sdi12 ]
    refresh: 1min

uart:
  - id: sdi12uart
    tx_pin:
      number: GPIO26
      inverted: true
    rx_pin: GPIO32
    baud_rate: 1200
    data_bits: 7
    parity: even
    half_duplex: true
    debug:
      direction: BOTH
      after:
        timeout: 20ms
      sequence:
        - lambda: UARTDebug::log_string(direction, bytes);

sdi12:
  - uart_id: sdi12uart
    id: sdibus

# Button configuration
binary_sensor:
  - platform: gpio
    pin: GPIO39
    name: "Physical Button"
    id: physical_button
    on_click:
      min_length: 3s
      max_length: 10s
      then:
        - switch.toggle: restart_switch

# Restart switch
switch:
  - platform: restart
    name: "Restart Device"
    id: restart_switch

sensor:
  - platform: uptime
    name: Uptime
    
  - platform: sdi12
    address: 0
    update_interval: 10s
    sensors:
      - index: 3
        name: pwEC
        id: pwec
        accuracy_decimals: 2
        unit_of_measurement: 'EC (ppm500)'
        filters:
          -  lambda: return x / 1000;
      - index: 2
        name: Temperature
        id: temperature
        accuracy_decimals: 1
        unit_of_measurement: '°C'
      - index: 1
        name: vWC
        id: vwc
        accuracy_decimals: 2
        unit_of_measurement: '%'
        filters:
          - lambda: |-
              float RAW = x;
              float vwc = 6.771e-10 * RAW * RAW * RAW - 5.105e-6 * RAW * RAW + 1.302e-2 * RAW - 10.848;
              vwc = vwc * 0.8;  // Scale from 0-100 to 0-80
              return vwc * 100;  // Convert to percentage
```

## Home Assistant Configuration for Arduino (Old version doesn't work with ESPoE)

Add the following to your `configuration.yaml`:

```yaml
mqtt:
  sensor:
    - name: "TEROS USA 1-5 Raw VWC"
      unique_id: "teros_usa_1_5_raw_vwc"
      state_topic: "sdi12/teros-usa-1-5"
      value_template: "{{ value_json.raw_vwc }}"
      unit_of_measurement: "Raw VWC"

    - name: "TEROS USA 1-5 VWC"
      unique_id: "teros_usa_1_5_vwc"
      state_topic: "sdi12/teros-usa-1-5"
      value_template: "{{ value_json.vwc }}"
      unit_of_measurement: "%"

    - name: "TEROS USA 1-5 Temperature"
      unique_id: "teros_usa_1_5_temperature"
      state_topic: "sdi12/teros-usa-1-5"
      value_template: "{{ value_json.temperature }}"
      unit_of_measurement: "°C"

    - name: "TEROS USA 1-5 Bulk EC"
      unique_id: "teros_usa_1_5_bulk_ec"
      state_topic: "sdi12/teros-usa-1-5"
      value_template: "{{ value_json.bulk_ec }}"
      unit_of_measurement: "dS/m"

    - name: "TEROS USA 1-5 Temp Comp EC"
      unique_id: "teros_usa_1_5_temp_comp_ec"
      state_topic: "sdi12/teros-usa-1-5"
      value_template: "{{ value_json.temp_comp_ec }}"
      unit_of_measurement: "dS/m"

    - name: "TEROS USA 1-5 Pore Water EC"
      unique_id: "teros_usa_1_5_pore_water_ec"
      state_topic: "sdi12/teros-usa-1-5"
      value_template: "{{ value_json.pore_water_ec }}"
      unit_of_measurement: "dS/m"

    - name: "TEROS USA 1-5 Saturation Extract EC"
      unique_id: "teros_usa_1_5_saturation_extract_ec"
      state_topic: "sdi12/teros-usa-1-5"
      value_template: "{{ value_json.saturation_extract_ec }}"
      unit_of_measurement: "dS/m"

    - name: "TEROS China 1-5-2 Raw VWC"
      unique_id: "teros_china_1_5_2_raw_vwc"
      state_topic: "sdi12/teros-china-1-5-2"
      value_template: "{{ value_json.raw_vwc }}"
      unit_of_measurement: "Raw VWC"

    - name: "TEROS China 1-5-2 VWC"
      unique_id: "teros_china_1_5_2_vwc"
      state_topic: "sdi12/teros-china-1-5-2"
      value_template: "{{ value_json.vwc }}"
      unit_of_measurement: "%"

    - name: "TEROS China 1-5-2 Temperature"
      unique_id: "teros_china_1_5_2_temperature"
      state_topic: "sdi12/teros-china-1-5-2"
      value_template: "{{ value_json.temperature }}"
      unit_of_measurement: "°C"

    - name: "TEROS China 1-5-2 Bulk EC"
      unique_id: "teros_china_1_5_2_bulk_ec"
      state_topic: "sdi12/teros-china-1-5-2"
      value_template: "{{ value_json.bulk_ec }}"
      unit_of_measurement: "dS/m"

    - name: "TEROS China 1-5-2 Temp Comp EC"
      unique_id: "teros_china_1_5_2_temp_comp_ec"
      state_topic: "sdi12/teros-china-1-5-2"
      value_template: "{{ value_json.temp_comp_ec }}"
      unit_of_measurement: "dS/m"

    - name: "TEROS China 1-5-2 Pore Water EC"
      unique_id: "teros_china_1_5_2_pore_water_ec"
      state_topic: "sdi12/teros-china-1-5-2"
      value_template: "{{ value_json.pore_water_ec }}"
      unit_of_measurement: "dS/m"

    - name: "TEROS China 1-5-2 Saturation Extract EC"
      unique_id: "teros_china_1_5_2_saturation_extract_ec"
      state_topic: "sdi12/teros-china-1-5-2"
      value_template: "{{ value_json.saturation_extract_ec }}"
      unit_of_measurement: "dS/m"

    - name: "TEROS China 1-2 Raw VWC"
      unique_id: "teros_china_1_2_raw_vwc"
      state_topic: "sdi12/teros-china-1-2"
      value_template: "{{ value_json.raw_vwc }}"
      unit_of_measurement: "Raw VWC"

    - name: "TEROS China 1-2 VWC"
      unique_id: "teros_china_1_2_vwc"
      state_topic: "sdi12/teros-china-1-2"
      value_template: "{{ value_json.vwc }}"
      unit_of_measurement: "%"

    - name: "TEROS China 1-2 Temperature"
      unique_id: "teros_china_1_2_temperature"
      state_topic: "sdi12/teros-china-1-2"
      value_template: "{{ value_json.temperature }}"
      unit_of_measurement: "°C"

    - name: "TEROS China 1-2 Bulk EC"
      unique_id: "teros_china_1_2_bulk_ec"
      state_topic: "sdi12/teros-china-1-2"
      value_template: "{{ value_json.bulk_ec }}"
      unit_of_measurement: "dS/m"

    - name: "TEROS China 1-2 Temp Comp EC"
      unique_id: "teros_china_1_2_temp_comp_ec"
      state_topic: "sdi12/teros-china-1-2"
      value_template: "{{ value_json.temp_comp_ec }}"
      unit_of_measurement: "dS/m"

    - name: "TEROS China 1-2 Pore Water EC"
      unique_id: "teros_china_1_2_pore_water_ec"
      state_topic: "sdi12/teros-china-1-2"
      value_template: "{{ value_json.pore_water_ec }}"
      unit_of_measurement: "dS/m"

    - name: "TEROS China 1-2 Saturation Extract EC"
      unique_id: "teros_china_1_2_saturation_extract_ec"
      state_topic: "sdi12/teros-china-1-2"
      value_template: "{{ value_json.saturation_extract_ec }}"
      unit_of_measurement: "dS/m"

```

### Home Assistant Card

Here's a Home Assistant card that uses the `multiple-entity-row` and `fold-entity-row` HACS cards:

![Home Assistant Card](https://github.com/JakeTheRabbit/HAGR/assets/123831499/6714a9b4-8ddf-495c-a57c-3325b54895a2)

```yaml
type: entities
title: TEROS Sensors
show_header_toggle: false
entities:

  - type: custom:fold-entity-row
    head:
      type: section
      label: "Grouped by Sensor"
    open: true
    items:
      - type: custom:multiple-entity-row
        entity: sensor.teros_china_1_2_pore_water_ec
        name: "TEROS China 1-2"
        entities:
          - entity: sensor.teros_china_1_2_vwc
            name: "VWC"
          - entity: sensor.teros_china_1_2_temperature
            name: "Temperature"

      - type: custom:multiple-entity-row
        entity: sensor.teros_china_1_5_2_pore_water_ec
        name: "TEROS China 1-5-2"
        entities:
          - entity: sensor.teros_china_1_5_2_vwc
            name: "VWC"
          - entity: sensor.teros_china_1_5_2_temperature
            name: "Temperature"

      - type: custom:multiple-entity-row
        entity: sensor.teros_usa_1_5_pore_water_ec
        name: "TEROS USA 1-5"
        entities:
          - entity: sensor.teros_usa_1_5_vwc
            name: "VWC"
          - entity: sensor.teros_usa_1_5_temperature
            name: "Temperature"
  - type: custom:fold-entity-row
    head:
      type: section
      label: "TEROS China 1-2"
    items:
      - entity: sensor.teros_china_1_2_pore_water_ec
        name: "Pore Water EC"
      - entity: sensor.teros_china_1_2_vwc
        name: "VWC"
      - entity: sensor.teros_china_1_2_temperature
        name: "Temperature"
      - entity: sensor.teros_china_1_2_bulk_ec
        name: "Bulk EC"
      - entity: sensor.teros_china_1_2_raw_vwc
        name: "Raw VWC"
      - entity: sensor.teros_china_1_2_saturation_extract_ec
        name: "Saturation Extract EC"
      - entity: sensor.teros_china_1_2_temp_comp_ec
        name: "Temp Comp EC"

  - type: custom:fold-entity-row
    head:
      type: section
      label: "TEROS China 1-5-2"
    items:
      - entity: sensor.teros_china_1_5_2_pore_water_ec
        name: "Pore Water EC"
      - entity: sensor.teros_china_1_5_2_vwc
        name: "VWC"
      - entity: sensor.teros_china_1_5_2_temperature
        name: "Temperature"
      - entity: sensor.teros_china_1_5_2_bulk_ec
        name: "Bulk EC"
      - entity: sensor.teros_china_1_5_2_raw_vwc
        name: "Raw VWC"
      - entity: sensor.teros_china_1_5_2_saturation_extract_ec
        name: "Saturation Extract EC"
      - entity: sensor.teros_china_1_5_2_temp_comp_ec
        name: "Temp Comp EC"

  - type: custom:fold-entity-row
    head:
      type: section
      label: "TEROS USA 1-5"
    items:
      - entity: sensor.teros_usa_1_5_pore_water_ec
        name: "Pore Water EC"
      - entity: sensor.teros_usa_1_5_vwc
        name: "VWC"
      - entity: sensor.teros_usa_1_5_temperature
        name: "Temperature"
      - entity: sensor.teros_usa_1_5_bulk_ec
        name: "Bulk EC"
      - entity: sensor.teros_usa_1_5_raw_vwc
        name: "Raw VWC"
      - entity: sensor.teros_usa_1_5_saturation_extract_ec
        name: "Saturation Extract EC"
      - entity: sensor.teros_usa_1_5_temp_comp_ec
        name: "Temp Comp EC"
```
You can also use template sensors in your `configuration.yaml` to simplify sensor naming and make it easier to update automations. Add the following to your `configuration.yaml`:

```yaml
sensor:
  - platform: template
    sensors:
      1_5_vwc:
        friendly_name: "1.5 VWC Sensor"
        value_template: "{{ states('sensor.teros_china_1_5_2_vwc') }}"
        unit_of_measurement: "%"
        icon_template: "mdi:water-percent"

      1_5_ec:
        friendly_name: "1.5 EC Sensor"
        value_template: "{{ states('sensor.teros_china_1_5_2_pore_water_ec') }}"
        unit_of_measurement: "dS/m"
        icon_template: "mdi:flash"

  - platform: template
    sensors:
      1_5_vwc_2:
        friendly_name: "1.5 VWC Sensor 2"
        value_template: "{{ states('sensor.teros_usa_1_5_vwc') }}"
        unit_of_measurement: "%"
        icon_template: "mdi:water-percent"

      1_5_ec_2:
        friendly_name: "1.5 EC Sensor 2"
        value_template: "{{ states('sensor.teros_usa_1_5_pore_water_ec') }}"
        unit_of_measurement: "mdS/m"
        icon_template: "mdi:flash"
```
## Sensor Comparison

Before the most recent iteration, some scientific testing was conducted to compare the EC and VWC readings of the Teros 12 Solus (benchmark) vs the Teros 12 USA and the Teros 12 China connected by ESP32. Note that the ESP32-only solution isn't perfect; there were slight discrepancies between the Solus data and the ESP32 data, but not significant enough to matter for the purposes of this project.

After observing a substantial difference in EC readings (the Teros 12 uses pore water EC, whereas the Chinese version outputs raw EC similar to an EC pen), calibrations were added using the Teros 12 manual (pages 15-16-17) https://www.labcell.com/media/140632/teros12%20manual.pdf

The numbers obtained using the pwEC calibration from the manual are very similar and follow the same trend lines on the graph as the USA Teros 12. This calibration could also be implemented in Home Assistant using template sensors for easier tweaking.

## Web Interface

A webserver has been added to the ESP32 sketch. An attempt was made to add OTA (Over-The-Air) updates, but it caused issues with MQTT, so it was omitted. You can view the live readings with a timestamp at the ESP32 device's IP address on your network:

![Web Interface](https://github.com/JakeTheRabbit/HAGR/assets/123831499/16d8e5f2-2c06-455c-91d6-cc21170beb43)

## Arduino Sketch

The Arduino sketch for this project can be found [here](https://github.com/JakeTheRabbit/HAGR/blob/main/teros-12/teros-12-sketch.ino).

## Contributing

If you find any issues or have suggestions for improvements, please open an issue or submit a pull request.

## License

This project is open-source and available under the [MIT License](LICENSE).

