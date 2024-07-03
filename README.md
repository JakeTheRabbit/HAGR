# HAGR
Home Assistant Grow Room

Here is a repository for grow automation related things for Home Assistant.

Some things might not work, there are basically no instructions. Its all pretty self explanatory if you are familiar with home assistant. 

Features integrated into Home Assistant:

This list is not exhaustive and I'm in the process of consolodating multiple setups into one shiny finished config and uploading here as I work through it.

- Co2 control with setpoints for day/night in Node Red using an SCD-41 and ESPAtom
- ESP32 Thermal Camera Leaf Temperature live readings into HA using MLX90641
- Live leaf and environment VPD calculations for ideal humidity target to maintain optimal VPD with varying temperature.
- Automatic VPD control (Leaf VPD) - set the target VPD and the automation will hold the leaf VPD steady with temp and humidity fluctuations
- Integrate THC-S, Teros 12 and the Alibaba Teros 12 Compatable VWC/EC Sensors
- Dosing nute tank with peristaltic pumps and Athena pro line nutes
- LED lights and drivers controlled with PWM as dimmable light entities in HA
- Tank level using Ultrasonic distance sensor
- Notifications
  - Hourly sensor updates
  - Alerts for low/high setpoints i.e. Temperature, VWC, CO2
- Automatic fan speed control
- ESP32 Controlled AC wall unit
- Irrigation Strategy / Crop Steering (still need to upload)
  - Multiple sensor triggers for redundancy without steering to an average
  - P1 Dosing and auto shot size calculator based off %
  - P1 substrate reset
  - P2 Minx/Max dosing and automatic field capacity adjustment
  - P3 emergency shot
  - P0 dryback % calculated based on current p3 VWC
  - Various transitions and fail safes
- Automatic lights on/off
- Day/night setpoints for temperature control
- Low CO2 auto light dimming

Home Assistant Addons: 
- ESP Home
- SSH & Web Terminal
- File Editor
- Samba
- Home Assistant Google Drive backup (do this first before)
- InfluxDB
- Grafana
- Node-Red
- Mosquito Broker


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

HACS Frontend:
 - Mushroom
 - Multiple Entity Row
 - Notify Card
 - Slider Button Card
 - Layout-card
 - Card-mod
 - VPD Chart card (lags out your HA)
 - Slider-entity-row
 - Mushroom - Better Sliders
 - Plotly Graph Card
 - card-tools
 - mini-graph-card

Useful links: 
ESPHome sensors from M5stack.com with configuration files: https://github.com/Chill-Division/M5Stack-ESPHome
A non m5stack way to setup the THC-S sensor  https://github.com/kromadg/soil-sensor
Killerherts HA growing Github https://github.com/Killerherts/nodeRed-HA-GrowingFunctions


