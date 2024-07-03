
I usually try and connect all my IoT devices through ESP Home to work with Home Assistant however the Arduino libraries for all the m5stack esp32 devices are far more comprehensive than the Esphome. Thankfully we have MQTT. 

This whole thing was made using Chat GPT/Claude I don't really have a working understanding so don't ask me for help. 

References/Resources I fed the GPT: 
- https://github.com/HarveyBates/ESP32-SDI12
- https://www.labcell.com/media/140632/teros12%20manual.pdf
- https://github.com/faiz-shukri/teros-12

How to setup: (this assumes you have basic knowledge of using Arduino IDE (ask Chat GPT for help its really good at it): 

1. Download Arduino IDE
2. Download https://github.com/HarveyBates/ESP32-SDI12 and install the library through zip in the arduino IDE. This enables SDI-12 communication directly with the ESP32.
3. Get an m5 stack grove cable and connect the cables see below image for what I did.
4. Get your ESP32 (I have tested with M5 Atom, M5 Atom S3, M5 PoEESP32 and M5 Dial).
5. Connect the Teros China 12 to the ESP32 and plug into your computer
7. Copy the .ino file into the Arduino IDE and flash the ESP32. Update all the variables at the top of ino and I think you need to download the .h and .cpp files as well. 
8. Any errors paste into chat gpt or claude for trouble shooting
9. Profit?


If you have a Teros-12 Solus with the 3.5mm jack this is the pinout:

<img width="1109" alt="image" src="https://github.com/JakeTheRabbit/HAGR/assets/123831499/6e8107e2-4be7-4fe8-b744-577949a88612">
<img width="744" alt="image" src="https://github.com/JakeTheRabbit/HAGR/assets/123831499/767efecf-1174-4a88-aeeb-df83142dacce">

If you want to use the Chinese Teros-12 rip off that also works with this code for about 1/3 the price. https://www.alibaba.com/product-detail/China-low-price-CE-IP68-SID12_1600643601689.html

Wiring for connecting the Chinese Teros to the Grove cable: 

If you are using the sensor in the Alibaba link and connecting to an M5 Stack ESP32 with a Grove cable: 

|Teros China|Grove Cable|
|-----------|---------|
|Red        | Yellow (Data) |
|White      | Red (Power)   |
|Ground wire - needs extra wrapping to isolate| Black (Ground) |
|N/A        | White just cut it off and isolate it    |

You can apply the same logic to the Teros 12 USA but ising a 3.5mm to 3 pin converter thing. 


<img width="444" alt="image" src="https://github.com/JakeTheRabbit/HAGR/assets/123831499/e233d3d0-7c3c-494e-95d3-3f13e804ed6c">

Bit of shrink wrap and thats it...

<img width="1003" alt="image" src="https://github.com/JakeTheRabbit/HAGR/assets/123831499/47168f2c-3daa-4163-83a8-019859e2bcde">


Assumming you have mqtt setup in Home Assistant add this to your configuration.yaml file. Ive got two of the chinese ones and one of the Teros 12s.

state_topic must match watch you've configured in the esp32 .ino file when you flash the esp32. These need to be different for each device otherwise mqttt doesn't like it. 

```
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

Here is a home assistant card that uses the multiplle-entity-row and fold-entity-row HACS cards. 

<img width="361" alt="image" src="https://github.com/JakeTheRabbit/HAGR/assets/123831499/6714a9b4-8ddf-495c-a57c-3325b54895a2">


```
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

<img width="952" alt="image" src="https://github.com/JakeTheRabbit/HAGR/assets/123831499/0b421f90-f178-4f13-826c-1d44bc067ab6">

You can also use template sensors in your configuration.yaml so that instead of having to go throughg all your automations and update sensor names you can just change which sensor names you are using and home assistant renames them. This goes in your configuration.yaml as well. 

```
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

Before the most recent iteration I did some scientific testing to see how the EC and VWC compared with the Teros 12 Solus (benchmark) vs the Teros 12 USA and the Teros 12 China connected by ESP32. One thing I will say is that the ESP32 only solution isn't perfect there were slight discrepancies between using the Solus data and the ESP32 data but for my purposes not enough to matter. After seeing the massive difference in the EC (the Teros 12 uses pore water EC I believe) whereas the Chinese just output raw EC (like what an EC pen would read) I used the Teros 12 manual to add some calibrations.
I have used the Teros12 manual pages 15 16 17 https://www.labcell.com/media/140632/teros12%20manual.pdf

The numbers I'm getting using the pwEC calibration from the manual are very similar and are following the same lines on the graph as the USA Teros 12. You could do this in Home Assistant as well with tempalate sensor would be easier to tweak that way. 

This was the initial readings before I added calibration and extra EC end points:  

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


Added a webserver, tried adding OTA but it was shitting bricks with MQTT so left it out. You can view the readings live with a timestamp at the ESP32 device IP address on your network: 

<img width="603" alt="image" src="https://github.com/JakeTheRabbit/HAGR/assets/123831499/16d8e5f2-2c06-455c-91d6-cc21170beb43">


Arduino .ino file: https://github.com/JakeTheRabbit/HAGR/blob/main/teros-12/teros-12-sketch.ino



