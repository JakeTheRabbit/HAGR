# ESPHome configs

Device firmware definitions for the sensor and actuator nodes used in the build.

> **Before you flash:** copy [`secrets.yaml.example`](secrets.yaml.example) to `secrets.yaml` and fill in your WiFi creds, API encryption key, and OTA password. Some configs in this directory still have those values inline — swap them for `!secret` references before flashing anything for real. [ESPHome secrets docs](https://esphome.io/guides/faq.html#how-do-i-use-secrets-passwords-in-yaml).

## Environmental sensing

| Config | Hardware | What it measures |
|---|---|---|
| [`CO2_Sensor_espatoms3.yaml`](CO2_Sensor_espatoms3.yaml) | M5Stack Atom S3 Lite + SCD41 (Grove) | CO2, temperature, RH, VPD. Roaming/handheld form factor. |
| [`SCD4x_m5stack_ESPOE.yaml`](SCD4x_m5stack_ESPOE.yaml) | M5Stack ESPPoE + SCD41 | CO2, temperature, RH on PoE. Daily min/max template sensors + CO2 alerts. |
| [`esphome_m5stack_scd41.yaml`](esphome_m5stack_scd41.yaml) | M5Stack Atom + SCD41 | Compact CO2 + temperature + RH monitor. Self-documented header. |
| [`m5Stack_AirQ.yaml`](m5Stack_AirQ.yaml) | M5Stack AirQ | Dual CO2 sensors with auto-calibration, PM (1/2.5/10), temperature, RH. |
| [`thermal_camera_leaf_temp.yaml`](thermal_camera_leaf_temp.yaml) | M5Stack Atom + MLX90614 | Single-pixel IR for direct leaf-surface temperature — input to the leaf-VPD calculation. |
| [`espatom_mlx90640.yaml`](espatom_mlx90640.yaml) | M5Stack Atom + MLX90640 | 32 × 24 thermal IR array for canopy temperature mapping. |

## Substrate and fertigation sensing

| Config | Hardware | What it measures |
|---|---|---|
| [`teros-12-sdi-12.yaml`](teros-12-sdi-12.yaml) | M5Stack Atom (ESP32) + SDI-12 interface | VWC, pore-water EC, substrate temperature from a TEROS-12 / compatible SDI-12 sensor. Calibration + pwEC math in templates. |
| [`m5stack_dial_ec_tdr_sdi12.yaml`](m5stack_dial_ec_tdr_sdi12.yaml) | M5Stack Dial + SDI-12 | Multi-sensor SDI-12 logger with a dial display. |
| [`atlas_wifi_hydroponics_kit.yaml`](atlas_wifi_hydroponics_kit.yaml) | EZO ESP32 hydroponics kit (Atlas Scientific) | pH, EC, RTD (temperature) for the reservoir or batch tank. |
| [`terralink.yaml`](terralink.yaml) | Growlink Terralink + M5Atom Lite | Repurposes a Terralink controller as an ESPHome node. |
| [`THC-S Home Assistant Template Sensors calibrated for Coco coir`](THC-S%20Home%20Assistant%20Template%20Sensors%20calibrated%20for%20Coco%20coir) | (HA template, not ESPHome) | Coco-coir-calibrated pwEC formula for the THC-S soil sensor. Paste into `templates:` in HA. |

## Lighting and actuation

| Config | Hardware | What it does |
|---|---|---|
| [`pwm_led_lights.yaml`](pwm_led_lights.yaml) | M5Stack Atom + M5 PWM unit | Replaces the potentiometer on a 0-10 V / PWM LED grow driver with HA-controllable PWM. Light is exposed as a native HA `light` entity. |
| [`espoe_peristaltic_pumps.yaml`](espoe_peristaltic_pumps.yaml) | M5Stack ESPPoE + relay/PWM channel | Peristaltic pump control for automated nutrient dosing. |
| [`m5Stack ESPPOE PB HUB 2CH-DualButton-BLEproxy.yaml`](m5Stack%20ESPPOE%20PB%20HUB%202CH-DualButton-BLEproxy.yaml) | M5Stack ESPPoE + PB Hub | Two-channel switch + dual physical button with BLE proxy. Useful as a multi-purpose plant-room node. |

## Tank and weight monitoring

| Config | Hardware | What it does |
|---|---|---|
| [`m5stack-dial-tank-monitor.yaml`](m5stack-dial-tank-monitor.yaml) + [README](m5stack-dial-tank-monitor-README.md) | M5Stack Dial + ultrasonic | Tank water level — distance and percentage on the dial, mounted outside the tank. |
| [`m5stack-dial-scales-co2-tank.yaml`](m5stack-dial-scales-co2-tank.yaml) | M5Stack Dial + load cell | CO2 cylinder weight — track consumption and refill needs. |

## Templates

- [`secrets.yaml.example`](secrets.yaml.example) — copy to `secrets.yaml` and fill in real values. Add `secrets.yaml` to `.gitignore`.

## Notes

- **PoE options** are flagged "ESPPoE" — they sidestep WiFi flakiness for permanent installs.
- **M5Stack ecosystem** is used throughout because the Grove connectors make sensor wiring plug-and-play. [Chill-Division/M5Stack-ESPHome](https://github.com/Chill-Division/M5Stack-ESPHome) is a great wider reference catalogue.
- **Substrate sensors:** the TEROS-12 path in this repo works but is fiddly. For a calibrated drop-in alternative use [Chill-Division/sdi12-substrate-sensor](https://github.com/Chill-Division/sdi12-substrate-sensor) — auto-discovers via MQTT.
