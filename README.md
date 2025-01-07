# HAGR: Home Assistant Grow Room

Welcome to the Home Assistant Grow Room (HAGR) repository. This project focuses on automating grow rooms using Home Assistant. Note that some features may not work as expected and instructions are minimal. Familiarity with Home Assistant is recommended.

Here is an exmaple of my fertigation control dashboard. 

<img width="825" alt="Irrigation Control v2" src="https://github.com/user-attachments/assets/773845f0-8afe-4255-b620-9c46b1f75d23" />


For more information on using the Teros-12 / Teros-12 Compatible SDI-12 with ESP32, visit: Teros-12 / Teros-12 Compatible SDI-12 to ESP32.

My Grow SOPs: 

- Cloning/Propagation: [Cloning Guide](growingSOPs/cloning.md)
- Environment cheat sheet: [Environmental Parameters](growingSOPs/Environment%20reference%20guide.md)
- Air flow for indoor grow environments: [Air flow requirements](growingSOPs/Indoor%20air%20flow.md)
- [Athena batch tank calculator (metric) hosted on github:](https://github.com/JakeTheRabbit/HAGR/blob/main/athena.html)
- <div></div>
  <img width="100" alt="image" src="https://github.com/user-attachments/assets/7394f98f-7d81-4b6b-bec8-78a4e703786f" />


## Useful Links

- ESPHome sensors from M5stack.com used for indoor gardenining with configuration files: [Chill-Division/M5Stack-ESPHome](https://github.com/Chill-Division/M5Stack-ESPHome)
- Excellent Home Assistant and ESP Home crop steering / garden automation repo: https://github.com/jeemers/Homegrown-Assistant/blob/main/README.md
- THC-S: [kromadg/soil-sensor](https://github.com/kromadg/soil-sensor)
- Killerherts HA growing functions: [Killerherts/nodeRed-HA-GrowingFunctions](https://github.com/Killerherts/nodeRed-HA-GrowingFunctions)
- Awesome Crop Steering: https://github.com/Intergalactic-XYZ/awesome-cropsteering
- If you like raw SDI-12 sensors and mqtt, none of this Home Assistant fluff: https://github.com/cropsteering/OS-SDI12
- Teros-12 / Teros-12 Compatable SDI-12 to ESP32 https://github.com/JakeTheRabbit/HAGR/blob/main/teros-12/Readme.md
- Another (working SDI-12 substrate sensor) https://github.com/Chill-Division/sdi12-substrate-sensor

Resources for automations:
- Grodan Grow guide: https://www.grodan101.com/siteassets/downloads/downloads-na-101/grow-guide-2023/grow-guide---cannabis-edition-2024.pdf
- Athena Grow Guide: https://issuu.com/athenaag/docs/athena_hb_me
- Growlink Crop Steering: https://www.growlink.ag/crop-steering

Tutorials:
- How to splice wires for sensors the right way: https://www.youtube.com/watch?v=aTpYi5nYjO0

General Info:
- Airflow simulation in a cannabis grow room: https://youtu.be/TOYe9ZFVRy8?si=7zaU3VCEeO92pmpk&t=11
- Grow room HVAC guide (commcercial offering documentation but still useful): https://midwestmachinery.net/wp-content/uploads/2020/01/Ultimate-Grow-Room-HVAC-Guide.pdf
- HydroBuddy v1.100 : The First Free Open Source Hydroponic Nutrient Calculator Program Available Online: https://scienceinhydroponics.com/2016/03/the-first-free-hydroponic-nutrient-calculator-program-o.html

Links:
- Open THC https://github.com/openthc
- Farm OS: https://github.com/farmOS/farmOS
- Open Foam: https://www.openfoam.com/


## Features

The integrated features include but are not limited to:

- **[CO2 Control](blueprints/co2_control_and_alerts.yaml):** Setpoints for day/night, high / low alerting, safety off, hysteresis, auto-dim lights on low CO2
- [Teros-12 SDI-12 to ESP32](https://github.com/JakeTheRabbit/HAGR/blob/main/teros-12/Readme.md): Connect your Teros 12 or make a cheap chinese version and connect to Home Assistant with an ESP32
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

## Screenshots

<a href="https://github.com/JakeTheRabbit/HAGR/assets/123831499/673f8f7f-dcf0-4fb1-9b35-9f19ce383b5e" target="_blank">
    <img src="https://github.com/JakeTheRabbit/HAGR/assets/123831499/673f8f7f-dcf0-4fb1-9b35-9f19ce383b5e" alt="1_5_dashboard2" width="200">
</a>
<a href="https://github.com/JakeTheRabbit/HAGR/assets/123831499/847fe601-8523-45e5-9813-8af4eafff33d" target="_blank">
    <img src="https://github.com/JakeTheRabbit/HAGR/assets/123831499/847fe601-8523-45e5-9813-8af4eafff33d" alt="environment config" width="200">
</a>
<a href="https://github.com/JakeTheRabbit/HAGR/assets/123831499/f788d65a-31c6-459c-a08a-5912b7a1fba6" target="_blank">
    <img src="https://github.com/JakeTheRabbit/HAGR/assets/123831499/f788d65a-31c6-459c-a08a-5912b7a1fba6" alt="crop steering config" width="200">
</a>

Peri-pump automatic batch tank filling: 

<img width="1205" alt="image" src="https://github.com/user-attachments/assets/344c5632-e1da-451d-a06a-b8f062dcf741" />

Coming soon...


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

