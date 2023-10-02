# HAGR
Home Assistant Grow Room

Here is a repository for grow automation related things for Home Assistant.

Some things might not work, there are basically no instructions. Its all pretty self explanatory if you are familiar with home assistant. 

If you have the THC-S (a cheap version like Aroya, GrowLink coco/rockwool humidity/ec/temp for 100$) here is how to integrate it into Home Assistant with ESP Home: https://github.com/JakeTheRabbit/TDR-Sensor/blob/f01d88421085d922dd9abb14dc89586b4b1563c4/README.md


Features: 
- Co2 control with setpoints for day/night in Node Red using an SCD-41 and ESPAtom
- Live leaf Temperature using MLX90640
- Live leaf and environment VPD calculations for ideal humidity target to maintain optimal VPD with varying temperature.
- Multi time select irrigation events
- Coco coir and rockwool calibrations for the affordable THC-S sensor
- Crop steering triggered by a THC-S (TDR Kind of) sensor for P3 dryback, p1 ramp up and p2 maintenance. You can change setpoints on the dash. EC is controlled manually at the moment.
- Dosing nute tank with peristaltic pumps and Athena pro line nutes
- LED lights and drivers controlled with PWM as dimmable light entities in HA
- Automatic nute tank level calculator based off pump switch.
- Coco coir and rockwool calibrations for the affordable THC-S sensor

Home Assistant Addons: 
- ESP Home
- SSH & Web Terminal
- File Editor
- Home Assistant Google Drive backup (do this first before)
- InfluxDB
- Grafana (don't really use it, it was sucking too much time and the grafs dont refresh fast enough for my liking)
- MariaDB storing like a years worth of data instead of the default 10 days Home Assistant uses.
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
  - Zigbee Home Automation

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
THC-S Rockwool Calibration for Home Assistant ESP Home: https://github.com/JakeTheRabbit/TDR-Sensor/blob/f01d88421085d922dd9abb14dc89586b4b1563c4/README.md

