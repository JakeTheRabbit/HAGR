# HAGR: Home Assistant Grow Room

Welcome to the Home Assistant Grow Room (HAGR) repository. This project focuses on automating grow rooms using Home Assistant. Note that some features may not work as expected and instructions are minimal. Familiarity with Home Assistant is recommended.

## Useful Links

- ESPHome sensors from M5stack.com used for indoor gardenining with configuration files: [Chill-Division/M5Stack-ESPHome](https://github.com/Chill-Division/M5Stack-ESPHome)
- Excellent Home Assistant and ESP Home crop steering / garden automation repo: https://github.com/jeemers/Homegrown-Assistant/blob/main/README.md
- THC-S: [kromadg/soil-sensor](https://github.com/kromadg/soil-sensor)
- Killerherts HA growing functions: [Killerherts/nodeRed-HA-GrowingFunctions](https://github.com/Killerherts/nodeRed-HA-GrowingFunctions)
- Awesome Crop Steering: https://github.com/Intergalactic-XYZ/awesome-cropsteering
- If you like raw SDI-12 sensors and mqtt, none of this Home Assistant fluff: https://github.com/cropsteering/OS-SDI12
- Teros-12 / Teros-12 Compatable SDI-12 to ESP32 https://github.com/JakeTheRabbit/HAGR/blob/main/teros-12/Readme.md


## Features

The integrated features include but are not limited to:

- **CO2 Control**: Setpoints for day/night using Node-Red with SCD-41 and ESPAtom.
- **Thermal Camera**: Live leaf temperature readings using ESP32 and MLX90641 in Home Assistant.
- **VPD Calculations**: Live leaf and environment calculations to maintain optimal humidity and temperature.
- **Automatic VPD Control**: Adjusts leaf VPD to maintain steady conditions despite temperature and humidity fluctuations.
- **Sensor Integrations**: Includes THC-S, Teros 12, and Alibaba Teros 12 compatible VWC/EC sensors.
- **Nutrient Dosing**: Automated dosing with peristaltic pumps using Athena Pro line nutrients.
- **Lighting Control**: LED lights and drivers controlled via PWM as dimmable entities in Home Assistant.
- **Tank Level Monitoring**: Ultrasonic distance sensor for tank levels.
- **Notifications**: Hourly sensor updates and alerts for high/low setpoints (e.g., Temperature, VWC, CO2).
- **Fan Speed Control**: Automatically adjusts fan speed.
- **AC Control**: ESP32-controlled AC wall unit.
- **Irrigation Strategy & Crop Steering**: (still in progress)
  - Multiple sensor triggers for redundancy
  - Dosing calculations and adjustments
  - Emergency shots and dryback percentage calculations
  - Various transitions and fail-safes
- **Lighting Automation**: Automatic lights on/off.
- **Temperature Control**: Day/night setpoints.
- **CO2-Triggered Light Dimming**: Low CO2 auto light dimming.

## Home Assistant Addons

I use these Home Assistant Addons:

- ESPHome
- SSH & Web Terminal
- File Editor
- Samba
- Home Assistant Google Drive Backup
- InfluxDB
- Grafana
- Node-Red
- Mosquito Broker

## Integrations

Integrations:

- HACS
- AC Infinity
- Blue Iris NVR
- Bluetooth
- ESPHome
- Google Calendar
- HASS.Agent
- LocalTuya
- Node-Red Companion
- Passive BLE Monitor
- RuuviTag BLE
- TP-Link Kasa Smart
- Tuya
- Xiaomi BLE
- Zigbee2Mqtt

## HACS Frontend Components

- Mushroom
- Multiple Entity Row
- Notify Card
- Slider Button Card
- Layout-card
- Card-mod
- VPD Chart Card (may cause lag)
- Slider-entity-row
- Mushroom - Better Sliders
- Plotly Graph Card
- card-tools
- mini-graph-card

