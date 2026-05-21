# Parts list

The bill of materials behind this build. Where there's more than one option, the **bold** one is what's actually in use; the rest are known-good equivalents.

## Environmental control

### CO₂ injection

- **[Growshop NZ CO₂ regulator + solenoid set](https://growshop.co.nz/products/co2regulatorsolenoidset)** — or any equivalent CO₂ solenoid + regulator.

### CO₂ sensing

Any Sensirion SCD3x / SCD4x will do. Pick by form factor.

| Sensor | Form | Source |
|---|---|---|
| SCD30 | Bare PCB | [DigiKey – Sensirion SCD30](https://www.digikey.co.nz/en/products/detail/sensirion-ag/SCD30/8445334) |
| SCD40 | M5Stack Grove unit | [M5Stack – CO₂ unit (SCD40)](https://shop.m5stack.com/products/co2-unit-with-temperature-and-humidity-sensor-scd40) |
| **SCD41** | M5Stack Grove unit | [M5Stack – CO₂L unit (SCD41)](https://shop.m5stack.com/products/co2l-unit-with-temperature-and-humidity-sensor-scd41) |

### Smart plugs (HA-controllable)

Any Tuya or TP-Link Kasa works. In use: **[TP-Link Kasa KP303 multi-plug](https://www.pbtech.co.nz/product/SURTPL3020/TP-Link-Kasa-KP303-Smart-Wi-Fi-Surge-Strip-3-x-Out)**.

## Substrate sensing

- **TEROS-12** or a compatible SDI-12 sensor — see [`teros-12/Readme.md`](teros-12/Readme.md) for wiring, the ESP32 sketch, and calibration.
- **THC-S soil sensor** — the coco-coir-calibrated pwEC template lives in [`esphome/THC-S Home Assistant Template Sensors calibrated for Coco coir`](esphome/THC-S%20Home%20Assistant%20Template%20Sensors%20calibrated%20for%20Coco%20coir).
- **Recommended for new builds:** [Chill-Division SDI-12 substrate sensor](https://github.com/Chill-Division/sdi12-substrate-sensor) — calibrated and auto-discovers over MQTT, so you skip the whole calibration hassle.

## Fertigation

- **Atlas Scientific EZO modules** — pH, EC, RTD probes for reservoir and batch-tank monitoring. See [`esphome/atlas_wifi_hydroponics_kit.yaml`](esphome/atlas_wifi_hydroponics_kit.yaml).
- **Peristaltic pumps** — driven from an ESPPoE relay/PWM. See [`esphome/espoe_peristaltic_pumps.yaml`](esphome/espoe_peristaltic_pumps.yaml).
- **Ultrasonic distance sensor** for tank level — see [`esphome/m5stack-dial-tank-monitor.yaml`](esphome/m5stack-dial-tank-monitor.yaml) for the M5Stack Dial readout.
- **Load cell + HX711** for CO₂ cylinder weight — see [`esphome/m5stack-dial-scales-co2-tank.yaml`](esphome/m5stack-dial-scales-co2-tank.yaml).

## Microcontrollers and host boards

- **M5Stack Atom / Atom S3 Lite / Atom Lite** — compact ESP32 boards with Grove ports for plug-and-play sensors.
- **M5Stack ESPPoE** — ESP32 with PoE for permanent installs where you don't want to deal with flaky WiFi.
- **M5Stack AirQ** — purpose-built air-quality node.
- **M5Stack Dial** — round display + rotary encoder, good for at-tank readouts.

Most of the sensors here come from the [M5Stack sensor catalogue](https://shop.m5stack.com/collections/m5-sensor).

## Optical / thermal

- **MLX90614** (single-pixel IR) — direct leaf-surface temperature for leaf-VPD. See [`esphome/thermal_camera_leaf_temp.yaml`](esphome/thermal_camera_leaf_temp.yaml).
- **MLX90640** (32 × 24 thermal array) — canopy temperature mapping. See [`esphome/espatom_mlx90640.yaml`](esphome/espatom_mlx90640.yaml).

## Lighting

- **PWM-dimmable LED drivers** (0-10 V or PWM input) — wire the M5 PWM unit in place of the potentiometer. See [`esphome/pwm_led_lights.yaml`](esphome/pwm_led_lights.yaml).
