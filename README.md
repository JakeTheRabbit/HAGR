# HAGR — Home Assistant Grow Room

> **This repo is free. My 2am dryback debugging is not.** If it saved you a crop, a weekend, or a nervous breakdown — [buy the rabbit a bag of nutes](https://github.com/sponsors/JakeTheRabbit). If it didn't, keep your money. I respect a tight nutrient budget.

[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Home Assistant](https://img.shields.io/badge/Home%20Assistant-2024.10+-41BDF5?logo=home-assistant&logoColor=white)](https://www.home-assistant.io)
[![ESPHome](https://img.shields.io/badge/ESPHome-Compatible-black?logo=esphome&logoColor=white)](https://esphome.io)
[![AppDaemon](https://img.shields.io/badge/AppDaemon-Required%20(monitor)-orange)](https://appdaemon.readthedocs.io)
[![HACS](https://img.shields.io/badge/HACS-Default-41BDF5)](https://hacs.xyz)

A running, lived-in Home Assistant build for cannabis grow rooms. VPD-aware climate control, four-phase crop steering, automated batch-tank dosing, PWM-dimmable LEDs, ESPHome sensors everywhere, and consolidated alerting. It's also the front door to a wider set of related repos — controllers, calculators, sensor firmware, training material, infrastructure bits.

<img width="825" alt="Fertigation control dashboard" src="https://github.com/user-attachments/assets/773845f0-8afe-4255-b620-9c46b1f75d23" />

---

## Contents

- [Repository index](#repository-index)
  - [Automations](#automations)
  - [Blueprints](#blueprints)
  - [Packages](#packages)
  - [ESPHome configs](#esphome-configs)
  - [Add-ons](#add-ons)
  - [Hardware notes](#hardware-notes)
  - [Growing SOPs](#growing-sops)
  - [Root-level files](#root-level-files)
- [Related repositories](#related-repositories)
- [What it does](#what-it-does)
- [The HA stack](#the-ha-stack)
- [External resources](#external-resources)
- [Screenshots](#screenshots)
- [A note on secrets](#a-note-on-secrets)

---

## Repository index

### Automations

Path: [`automations/`](automations/)

Working Home Assistant automations you can copy in, swap entity IDs, and run. Drop them into a `packages/` directory or paste them into the UI editor.

**Safety and alerting**

| File | What it does |
|---|---|
| [`light_leak_detection.yaml`](automations/light_leak_detection.yaml) | The single most important automation in a flowering room. Any ambient-light sensor above 1 lux during the dark period kills the grow lights and fires a critical notification. Also packaged as a [blueprint](blueprints/light_leak_detection.yaml). |
| [`co2_multi_sensor_shutoff.yaml`](automations/co2_multi_sensor_shutoff.yaml) | Closes the CO₂ solenoid if any of N room sensors reads above 1800 ppm. Catches stratified pockets that single-sensor shutoff misses. |
| [`sensor_unavailability_watchdog.yaml`](automations/sensor_unavailability_watchdog.yaml) | Tells you when a temp, RH, or CO₂ sensor has been offline for ten minutes. Fail-safe closes the CO₂ solenoid if the CO₂ sensor itself dies. |
| [`daily_safe_state_audit.yaml`](automations/daily_safe_state_audit.yaml) | At 03:00 every night, force-closes every valve, the CO₂ solenoid, and the humidifier. Belt-and-braces sweep for stuck valves and forgotten overrides. |
| [`CO2_low_Alert.yaml`](automations/CO2_low_Alert.yaml) | Actionable Android alert when CO₂ drops 300 ppm below target for five minutes, or when the CO₂ valve has been open longer than five minutes (probable empty tank). Bypasses do-not-disturb. |

**Lighting and environment**

| File | What it does |
|---|---|
| [`light_acclimation.yaml`](automations/light_acclimation.yaml) | Ramps brightness from 50% to 90% over 21 days then turns itself off. Triggered at lights-on. |
| [`humidifier_off_at_lights_off.yaml`](automations/humidifier_off_at_lights_off.yaml) | Hard-pins the humidifier off when lights go off. Lets the dehumidifiers run the dark period uncontested. |
| [`Adjust ACI Fan Speed External Sensor.md`](automations/Adjust%20ACI%20Fan%20Speed%20External%20Sensor.md) | AC Infinity fan speed (0-10) from an external temp sensor with separate day and night targets. |
| [`Leaf_VPD.md`](automations/Leaf_VPD.md) | Leaf-VPD vs air-VPD, and the full control system that uses an IR leaf-temp sensor to drive a humidifier or dehumidifier. |

**Irrigation and dosing**

| File | What it does |
|---|---|
| [`refill_tank_triggered_by_low_level.yaml`](automations/refill_tank_triggered_by_low_level.yaml) | Full batch-tank refill plus Athena Pro Line dose sequence. Fires on low water level, manual button, or tank-empty signal. Pre-fill, refill to volume, dose A/B/C/Bloom/Core in sequence, mix delay, pH/EC verification, completion alert. |
| [`Automatic_batch_tank_filling_full_explainer.md`](automations/Automatic_batch_tank_filling_full_explainer.md) | Long-form walk-through of the refill and multi-part Athena dosing logic. |
| [`auto dosing athena.md`](automations/auto%20dosing%20athena.md) | Helper definitions (EC target, solution volume) and dosing-node sequence for an Athena auto-dose workflow. |
| [`crop steering node red flow`](automations/crop%20steering%20node%20red%20flow) | Older Node-RED export of a crop-steering implementation. Most people should use [`Packages/CropSteering/`](Packages/CropSteering/) instead. |

**Access control**

| File | What it does |
|---|---|
| [`door_open_siren.yaml`](automations/door_open_siren.yaml) | Sounds a siren and notifies operators if a critical door stays open longer than 20 seconds. Cancels when the door closes. |
| [`auto_door_lock.yaml`](automations/auto_door_lock.yaml) | Locks a door 20 seconds after it closes. Kills the "I forgot to lock it" failure mode. |

**CO₂ blueprint imports**

| File | What it does |
|---|---|
| [`co2_automation.MD`](automations/co2_automation.MD) | Import link and notes for the CO₂ control blueprint. See also [`blueprints/co2_control_and_alerts.yaml`](blueprints/co2_control_and_alerts.yaml). |

### Blueprints

Path: [`blueprints/`](blueprints/)

The same patterns as above, wrapped as Home Assistant blueprints. Import once, then point them at your entities from the UI.

| Blueprint | What it does |
|---|---|
| [`co2_control_and_alerts.yaml`](blueprints/co2_control_and_alerts.yaml) | Day/night CO₂ setpoints with hysteresis switching, high-CO₂ auto-off (primary + optional backup sensor, with control lockout), relay-stuck/empty-tank auto-off, sustained-low alerts with cooldown, and optional light auto-dim with automatic restore on recovery. |
| [`grow_room_env_threshold_alerts.yaml`](blueprints/grow_room_env_threshold_alerts.yaml) | One blueprint, all your environmental alerts. Separate day/night thresholds, pause switch, persistence delay, helper-backed cooldown between notifications, multi-device notify targets. |
| [`auto_temp_triggered_light_dimming.yaml`](blueprints/auto_temp_triggered_light_dimming.yaml) | When the room overheats, step-dim the lights until temp comes back, then restore them to their pre-dim brightness. Won't switch lights on during the dark period. Plant-safe heat protection. |
| [`light_leak_detection.yaml`](blueprints/light_leak_detection.yaml) | Critical light-leak alarm. Any illuminance sensor above the threshold while your lights-on binary sensor is off forces the grow lights off and runs your notify action — re-checked on restart and every minute, so a leak already present after a power blip is still caught. |

### Packages

Path: [`Packages/`](Packages/)

Larger feature sets bundled as Home Assistant packages. To use them, add this to your `configuration.yaml`:

```yaml
homeassistant:
  packages: !include_dir_named packages
```

| Folder | What's inside |
|---|---|
| [`Packages/CropSteering/`](Packages/CropSteering/) | A four-phase (P0/P1/P2/P3) crop-steering package — input helpers, template sensors, automations, and a long user guide. Start with [`Packages/ReadMe.MD`](Packages/ReadMe.MD) and [`crop_steering_installation_guide_improved.md`](Packages/CropSteering/crop_steering_installation_guide_improved.md). |

Two standalone YAMLs sit at the repo root and can be `!include`d into your own setup:

- [`crop_steering_package.yaml`](crop_steering_package.yaml) — an earlier single-file snapshot of the same crop-steering logic, kept for reference.
- [`vpd_configuration.yaml`](vpd_configuration.yaml) — leaf-VPD and air-VPD template sensors.

### ESPHome configs

Path: [`esphome/`](esphome/)

Device YAMLs for the sensor and actuator nodes used in the build. Each file's header explains wiring and any quirks.

| Config | Device | What it does |
|---|---|---|
| [`CO2_Sensor_espatoms3.yaml`](esphome/CO2_Sensor_espatoms3.yaml) | [M5Stack Atom S3 Lite](https://shop.m5stack.com/products/atoms3-lite-esp32s3-dev-kit) + [SCD41](https://shop.m5stack.com/products/co2l-unit-with-temperature-and-humidity-sensor-scd41) (Grove) | Roaming CO₂, temp, RH, VPD. |
| [`SCD4x_m5stack_ESPOE.yaml`](esphome/SCD4x_m5stack_ESPOE.yaml) | [M5Stack ESPPoE](https://shop.m5stack.com/products/m5stack-atom-poe-kit-with-w5500-hy601742e) + [SCD41](https://shop.m5stack.com/products/co2l-unit-with-temperature-and-humidity-sensor-scd41) | PoE-powered CO₂ sensor with VPD, daily min/max, and CO₂ alerts as template sensors. |
| [`esphome_m5stack_scd41.yaml`](esphome/esphome_m5stack_scd41.yaml) | [M5Stack Atom Lite](https://shop.m5stack.com/products/atom-lite-esp32-development-kit) + [SCD41](https://shop.m5stack.com/products/co2l-unit-with-temperature-and-humidity-sensor-scd41) | Compact CO₂ + environment monitor. |
| [`m5Stack_AirQ.yaml`](esphome/m5Stack_AirQ.yaml) | [M5Stack AirQ](https://shop.m5stack.com/products/atomic-airq-kit-w-co2-tvoc-eco2) | Dual CO₂ sensors with auto-calibration, PM, RH, temperature. |
| [`m5stack-dial-tank-monitor.yaml`](esphome/m5stack-dial-tank-monitor.yaml) ([README](esphome/m5stack-dial-tank-monitor-README.md)) | [M5Stack Dial](https://shop.m5stack.com/products/m5stack-dial-esp32-s3-smart-rotary-knob-w-1-28-round-touch-screen) + [ultrasonic](https://shop.m5stack.com/products/i2c-ultrasonic-distance-unit-rcwl-9620) | Tank water level — distance and percentage, displayed on the dial mounted outside the tank. |
| [`m5stack-dial-scales-co2-tank.yaml`](esphome/m5stack-dial-scales-co2-tank.yaml) | [M5Stack Dial](https://shop.m5stack.com/products/m5stack-dial-esp32-s3-smart-rotary-knob-w-1-28-round-touch-screen) + [load cell](https://shop.m5stack.com/products/mini-scales-unit-with-loadcell-sensor-hx711) | CO₂ cylinder weight — track consumption and know when to refill. |
| [`m5stack_dial_ec_tdr_sdi12.yaml`](esphome/m5stack_dial_ec_tdr_sdi12.yaml) | [M5Stack Dial](https://shop.m5stack.com/products/m5stack-dial-esp32-s3-smart-rotary-knob-w-1-28-round-touch-screen) + [SDI-12](https://sdi-12.org/) | Logger for SDI-12 substrate sensors (EC, VWC, temperature) with a dial readout. |
| [`teros-12-sdi-12.yaml`](esphome/teros-12-sdi-12.yaml) | [M5Stack Atom Lite](https://shop.m5stack.com/products/atom-lite-esp32-development-kit) ([ESP32](https://www.espressif.com/en/products/socs/esp32)) + [SDI-12](https://sdi-12.org/) | Reads [TEROS-12](https://www.metergroup.com/en/meter-environment/products/teros-12) and compatible SDI-12 soil sensors. Includes the calibration and pore-water EC math. |
| [`terralink.yaml`](esphome/terralink.yaml) | [Growlink Terralink](https://growlink.com/products/terralink-environmental-monitor/) + [M5Atom Lite](https://shop.m5stack.com/products/atom-lite-esp32-development-kit) | Turns a Growlink Terralink into an ESPHome node. |
| [`atlas_wifi_hydroponics_kit.yaml`](esphome/atlas_wifi_hydroponics_kit.yaml) | [Atlas Scientific EZO](https://atlas-scientific.com/embedded-solutions/) hydroponics kit on [ESP32](https://www.espressif.com/en/products/socs/esp32) | pH, EC, and RTD (temperature) from [Atlas Scientific EZO modules](https://atlas-scientific.com/probes/). |
| [`espoe_peristaltic_pumps.yaml`](esphome/espoe_peristaltic_pumps.yaml) | [M5Stack ESPPoE](https://shop.m5stack.com/products/m5stack-atom-poe-kit-with-w5500-hy601742e) + relay/PWM | Peristaltic pump control for automated dosing. |
| [`m5Stack ESPPOE PB HUB 2CH-DualButton-BLEproxy.yaml`](esphome/m5Stack%20ESPPOE%20PB%20HUB%202CH-DualButton-BLEproxy.yaml) | [M5Stack ESPPoE](https://shop.m5stack.com/products/m5stack-atom-poe-kit-with-w5500-hy601742e) + [PB Hub](https://shop.m5stack.com/products/pbhub-unit-v1-1) | Two-channel switch / dual-button with BLE proxy. |
| [`pwm_led_lights.yaml`](esphome/pwm_led_lights.yaml) | [M5Stack Atom Lite](https://shop.m5stack.com/products/atom-lite-esp32-development-kit) + [PWM unit](https://shop.m5stack.com/products/pwm-unit) | Dim a 0-10 V or PWM LED driver as a native HA `light` entity. Wired in place of the potentiometer. |
| [`espatom_mlx90640.yaml`](esphome/espatom_mlx90640.yaml) | [M5Stack Atom Lite](https://shop.m5stack.com/products/atom-lite-esp32-development-kit) + [MLX90640](https://shop.m5stack.com/products/thermal2-unit-mlx90640) | 32 × 24 thermal IR array for canopy temperature mapping. |
| [`thermal_camera_leaf_temp.yaml`](esphome/thermal_camera_leaf_temp.yaml) | [M5Stack Atom Lite](https://shop.m5stack.com/products/atom-lite-esp32-development-kit) + [MLX90614](https://shop.m5stack.com/products/mini-temperature-unit-mlx90614) | Single-pixel IR for direct leaf-surface temperature. Feeds the leaf-VPD calculation. |
| [`THC-S Home Assistant Template Sensors calibrated for Coco coir`](esphome/THC-S%20Home%20Assistant%20Template%20Sensors%20calibrated%20for%20Coco%20coir) | HA template (not ESPHome) | Coco-coir-calibrated pwEC formula for the [THC-S soil sensor](https://github.com/kromadg/soil-sensor). |
| [`secrets.yaml.example`](esphome/secrets.yaml.example) | — | Template for `secrets.yaml`. Copy and fill in WiFi credentials, API encryption key, and OTA password. |

### Add-ons

Path: [`addons/`](addons/)

| Add-on | What it does |
|---|---|
| [`addons/appdaemon/grow_monitor/`](addons/appdaemon/grow_monitor/) | **Grow Room Monitor** — an AppDaemon app that watches seven sensor types (temp, RH, CO₂, VPD, leaf VPD, VWC, pwEC) with day/night-aware thresholds and trend analysis. Sends one consolidated, severity-graded notification with mute and pause actions rather than spamming you per sensor. Optional OpenAI integration writes a short situation summary into the alert. See [`Readme.MD`](addons/appdaemon/grow_monitor/Readme.MD). |

### Hardware notes

Path: [`teros-12/`](teros-12/)

| File | What it does |
|---|---|
| [`teros-12/Readme.md`](teros-12/Readme.md) | TEROS-12 (and compatible SDI-12 clone) wiring, ESP32 sketch, calibration. A newer dedicated repo is [TDR-Sensor](https://github.com/JakeTheRabbit/TDR-Sensor). |
| [`teros-12/EsPoE-Teros-12.ino`](teros-12/EsPoE-Teros-12.ino) | Arduino sketch for the ESPPoE board reading TEROS-12 over SDI-12. |
| [`teros-12/teros-12-sketch.ino`](teros-12/teros-12-sketch.ino) | Standalone Arduino sketch — has its own web UI and MQTT publishing. |
| [`teros-12/libraries/ESP32-SDI12/`](teros-12/libraries/ESP32-SDI12/) | Bundled ESP32 SDI-12 library. |

### Growing SOPs

Path: [`growingSOPs/`](growingSOPs/)

| Document | Topic |
|---|---|
| [`cloning.md`](growingSOPs/cloning.md) | Cloning and propagation. |
| [`Environment reference guide.md`](growingSOPs/Environment%20reference%20guide.md) | Stage-by-stage setpoint cheat sheet — temperature, RH, VPD, CO₂, PPFD, DLI. |
| [`Derived crop signals.md`](growingSOPs/Derived%20crop%20signals.md) | Sensor-derived crop signals for leaf/air VPD, canopy cooling, dryback, EC trends, and Home Assistant templates. |
| [`Indoor air flow.md`](growingSOPs/Indoor%20air%20flow.md) | Airflow requirements and room layout for indoor grows. |
| [`Indoor Pathogens.md`](growingSOPs/Indoor%20Pathogens.md) | Common indoor cultivation pathogens — recognition, prevention, response. |
| [`Cleaning.md`](growingSOPs/Cleaning.md) | Cleaning and sanitation procedures. |

### Root-level files

| File | What it does |
|---|---|
| [`automated_batch_tank.md`](automated_batch_tank.md) | System diagram and the full automation for batch-tank fill and dose. |
| [`athena.html`](athena.html) | Standalone single-file Athena Pro Line batch-tank calculator. The hosted version lives at [`batch_tank_calculator`](https://github.com/JakeTheRabbit/batch_tank_calculator). |
| [`crop_steering_package.yaml`](crop_steering_package.yaml) | Earlier single-file crop-steering snapshot. For new installs use [`Packages/CropSteering/`](Packages/CropSteering/). |
| [`vpd_configuration.yaml`](vpd_configuration.yaml) | Leaf and air VPD template sensors. |
| [`PARTS_LIST.md`](PARTS_LIST.md) | Bill of materials — sensors, valves, plugs, microcontrollers, drivers. |

---

## Related repositories

The wider system, broken into focused repos. Some are reference implementations, some are the production code, some are companion tools.

### Crop steering and irrigation

| Repo | What it is |
|---|---|
| **[HA-Irrigation-Strategy](https://github.com/JakeTheRabbit/HA-Irrigation-Strategy)** | A proper multi-zone (1-6) crop-steering controller for Home Assistant. Custom integration plus AppDaemon modules. Phase state machine, VWC/EC-driven irrigation, full dashboards. The most actively developed crop-steering setup in this ecosystem. |
| **[open-crop-steering](https://github.com/JakeTheRabbit/open-crop-steering)** | A Home Assistant add-on (with a Docker fallback) that turns cultivation recipes into versioned, immutable plans. Adds an AI runtime layer bounded by hard guardrails, and produces tamper-evident audit records suitable for regulated facilities. v0.1, in development. |
| **[irrigation_unlimited](https://github.com/JakeTheRabbit/irrigation_unlimited)** | Fork of the Irrigation Unlimited HA integration (rgc99), pinned for the irrigation stack. |

### Calculators and tools

| Repo | What it is |
|---|---|
| **[batch_tank_calculator](https://github.com/JakeTheRabbit/batch_tank_calculator)** | Athena Pro Line batch-tank nutrient calculator — metric and imperial, scales to any reservoir volume, persists calculations to local storage, exports to CSV. **[Live site](https://jaketherabbit.github.io/Athena-Batch-Tank-Nutrient-Calculator/)**. |
| **[cannabis-grow-room-levers](https://github.com/JakeTheRabbit/cannabis-grow-room-levers)** | A systems-thinking field guide — the grow room treated as a coupled energy / moisture / carbon / salt / biology system. Eleven control levers, four balances, a lever-interaction matrix, a diagnostic chain, six common failure modes. Single self-contained HTML. **[Live site](https://jaketherabbit.github.io/cannabis-grow-room-levers/)**. |
| **[Grow Room Training](https://github.com/JakeTheRabbit/grow-room-training)** | Training material distilled from the levers framework — a visual manual, a 15-chapter textbook with auto-scored MCQs, printable field cards, and an LLM-agent playbook for autonomous HA control. |
| **[cultivation-intelligence](https://github.com/JakeTheRabbit/cultivation-intelligence)** | AI cultivation intelligence platform — keeps every sensor reading and irrigation event in a time-series history, with an LLM advisory layer on top. |

### Infrastructure and integrations

| Repo | What it is |
|---|---|
| **[mosquitto-bridge-setup](https://github.com/JakeTheRabbit/mosquitto-bridge-setup)** | Mosquitto bridge configuration for dual-boot Windows/Linux relaying. Useful when a no-auth nutrient doser has to talk to authenticated HA MQTT. |
| **[visitors_repo](https://github.com/JakeTheRabbit/visitors_repo)** | A small visitor sign-in / sign-out tracker for HA. Built to be quick and to not get tangled up when visitors forget to sign out. |
| **[rosinlab](https://github.com/JakeTheRabbit/rosinlab)** | Local-first rosin press and sift logging web app. Embeds inside Home Assistant via an iframe card. Compare mode plays up to three press videos side by side with parameters. |
| **[TDR-Sensor](https://github.com/JakeTheRabbit/TDR-Sensor)** | The newer dedicated repo for TEROS-12 and compatible SDI-12 substrate sensor builds. Supersedes the [`teros-12/`](teros-12/) folder here. |

---

## What it does

The stack covers:

- **CO₂ control** — day/night setpoints, hysteresis, safety auto-off, low and high alerting, and an option to auto-dim lights when CO₂ is sustained low. See [`blueprints/co2_control_and_alerts.yaml`](blueprints/co2_control_and_alerts.yaml).
- **VPD calculations** — live air VPD and leaf VPD via [`vpd_configuration.yaml`](vpd_configuration.yaml). Leaf VPD uses an [MLX90614](https://shop.m5stack.com/products/mini-temperature-unit-mlx90614) IR temperature reading.
- **Automatic VPD control** — drives a humidifier or dehumidifier to hold leaf VPD steady through temperature and RH drift.
- **Crop steering and irrigation** — full four-phase (P0/P1/P2/P3) package in [`Packages/CropSteering/`](Packages/CropSteering/). For larger multi-zone setups use [HA-Irrigation-Strategy](https://github.com/JakeTheRabbit/HA-Irrigation-Strategy).
- **Substrate sensing** — [TEROS-12](https://www.metergroup.com/en/meter-environment/products/teros-12) and [SDI-12](https://sdi-12.org/) sensors talking to an [ESP32](https://www.espressif.com/en/products/socs/esp32), giving you VWC and pore-water EC. See [`teros-12/`](teros-12/) and [`esphome/teros-12-sdi-12.yaml`](esphome/teros-12-sdi-12.yaml). If you want something that just works, use [Chill-Division/sdi12-substrate-sensor](https://github.com/Chill-Division/sdi12-substrate-sensor) — calibrated and auto-discovers over MQTT.
- **Nutrient dosing** — peristaltic pumps run from an [M5Stack ESPPoE](https://shop.m5stack.com/products/m5stack-atom-poe-kit-with-w5500-hy601742e), automated batch-tank fill, multi-part [Athena Pro Line](https://athenaag.com/) dosing, then a post-dose pH/EC verification through [Atlas Scientific EZO](https://atlas-scientific.com/probes/) probes. See [`automated_batch_tank.md`](automated_batch_tank.md) and [`automations/refill_tank_triggered_by_low_level.yaml`](automations/refill_tank_triggered_by_low_level.yaml).
- **Lighting** — 0-10 V or PWM-dimmable LED drivers exposed as native HA `light` entities through the [M5Stack PWM unit](https://shop.m5stack.com/products/pwm-unit). See [`esphome/pwm_led_lights.yaml`](esphome/pwm_led_lights.yaml). A linear PPFD acclimation ramp lives in [`automations/light_acclimation.yaml`](automations/light_acclimation.yaml).
- **Tank level monitoring** — ultrasonic distance sensor on the tank lid with the readout on an [M5Stack Dial](https://shop.m5stack.com/products/m5stack-dial-esp32-s3-smart-rotary-knob-w-1-28-round-touch-screen) mounted outside the tank. See [`esphome/m5stack-dial-tank-monitor.yaml`](esphome/m5stack-dial-tank-monitor.yaml).
- **Notifications** — consolidated environmental alerts with day/night thresholds, mute and pause actions, and an optional AI summary line. See [`addons/appdaemon/grow_monitor/`](addons/appdaemon/grow_monitor/).
- **Fan speed control** — [AC Infinity](https://acinfinity.com/) fans driven from an external temp sensor. See [`automations/Adjust ACI Fan Speed External Sensor.md`](automations/Adjust%20ACI%20Fan%20Speed%20External%20Sensor.md).
- **AC control** — IR-blaster control of a wall-mount split AC with PID, holding the room to about ±0.2 °C with regular hardware. See HA's [Broadlink](https://www.home-assistant.io/integrations/broadlink) and [SmartIR](https://github.com/smartHomeHub/SmartIR) integrations.
- **Thermal canopy temperature** — [ESP32](https://www.espressif.com/en/products/socs/esp32) with an [MLX90640](https://shop.m5stack.com/products/thermal2-unit-mlx90640) 32 × 24 thermal array, or an [MLX90614](https://shop.m5stack.com/products/mini-temperature-unit-mlx90614) single-pixel for spot leaf temp. See [`esphome/espatom_mlx90640.yaml`](esphome/espatom_mlx90640.yaml) and [`esphome/thermal_camera_leaf_temp.yaml`](esphome/thermal_camera_leaf_temp.yaml).

---

## The HA stack

Everything below is what actually runs the build. Each entry links to its source.

### Add-ons

| Add-on | Source | Notes |
|---|---|---|
| [AppDaemon](https://github.com/hassio-addons/addon-appdaemon) | Community Add-ons | Hosts [`addons/appdaemon/grow_monitor/`](addons/appdaemon/grow_monitor/) and the HA-Irrigation-Strategy AppDaemon modules. |
| [ESPHome Device Builder](https://github.com/esphome/home-assistant-addon) | Official | Flashes and manages every sensor and actuator in [`esphome/`](esphome/). |
| [Frigate NVR](https://github.com/blakeblackshear/frigate-hass-addons) | Frigate | Object-detection NVR. |
| [Matter Server](https://github.com/home-assistant/addons) | Official | Matter device support. |
| [Mosquitto Broker](https://github.com/home-assistant/addons) | Official | MQTT broker. Used by ESPHome, Zigbee2MQTT, and the custom doser. |
| [NetBird](https://github.com/netbirdio/home-assistant-addons) | NetBird | WireGuard-based mesh VPN. |
| [Node-RED](https://github.com/hassio-addons/addon-node-red) | Community Add-ons | Used by [`automations/crop steering node red flow`](automations/crop%20steering%20node%20red%20flow). |
| [OpenClaw Assistant](https://github.com/techartdev/OpenClawHomeAssistant) | OpenClaw | Companion add-on for the OpenClaw integration. |
| [Whisper](https://github.com/home-assistant/addons) | Official | Local speech-to-text for the voice assistant pipeline. |
| [Z-Wave JS](https://github.com/home-assistant/addons) | Official | Z-Wave controller. |
| [Zigbee2MQTT](https://github.com/zigbee2mqtt/hassio-zigbee2mqtt) | Z2M | Zigbee bridge. Runs in parallel with ZHA on a second radio. |
| [Visitors](https://github.com/JakeTheRabbit/visitors_repo) | This org | Visitor sign-in / sign-out web app. |

### Core integrations

Integrations you actually have to set up — `default_config` auto-loaders aren't listed.

**Grow-room directly relevant**

| Integration | Source / docs |
|---|---|
| [AC Infinity](https://github.com/dalinicus/homeassistant-acinfinity) | HACS custom integration |
| [Better Thermostat](https://github.com/KartoffelToby/better_thermostat) | HACS custom integration |
| [Crop Steering System](https://github.com/JakeTheRabbit/HA-Irrigation-Strategy) | HACS custom integration |
| [ESPHome](https://www.home-assistant.io/integrations/esphome) | Core integration |
| [Frigate](https://github.com/blakeblackshear/frigate-hass-integration) | HACS custom integration |
| [Generic Hygrostat](https://www.home-assistant.io/integrations/generic_hygrostat) | Core integration |
| [Generic Thermostat](https://www.home-assistant.io/integrations/generic_thermostat) | Core integration |
| [HVAC Group](https://github.com/tetele/hvac_group) | HACS custom integration |
| [InfluxDB](https://www.home-assistant.io/integrations/influxdb) | Core integration |
| [Local Calendar](https://www.home-assistant.io/integrations/local_calendar) | Core integration |
| [Local To-do](https://www.home-assistant.io/integrations/local_todo) | Core integration |
| [MQTT](https://www.home-assistant.io/integrations/mqtt) | Core integration |
| [MetService NZ Weather](https://github.com/ciejer/metservice-weather) | HACS custom integration |
| [Mobile App](https://www.home-assistant.io/integrations/mobile_app) | Core integration |
| [Powercalc](https://github.com/bramstroker/homeassistant-powercalc) | HACS custom integration |
| [Scheduler](https://github.com/nielsfaber/scheduler-component) | HACS custom integration |
| [Template](https://www.home-assistant.io/integrations/template) | Core integration |
| [ZHA](https://www.home-assistant.io/integrations/zha) | Core integration |
| [Zigbee2MQTT](https://github.com/zigbee2mqtt/hassio-zigbee2mqtt) | Add-on + [MQTT integration](https://www.home-assistant.io/integrations/mqtt) |
| [Z-Wave JS](https://www.home-assistant.io/integrations/zwave_js) | Core integration |

**The rest of the house**

<details>
<summary>Click to expand</summary>

| Integration | Source / docs |
|---|---|
| [Alarmo](https://github.com/nielsfaber/alarmo) | HACS custom integration |
| [Alexa Media Player](https://github.com/alandtse/alexa_media_player) | HACS custom integration |
| [Ambient Network](https://www.home-assistant.io/integrations/ambient_network) | Core integration |
| [Anthropic](https://www.home-assistant.io/integrations/anthropic) / [OpenAI](https://www.home-assistant.io/integrations/openai_conversation) / [Grok](https://github.com/braytonstafford/grok_conversation) / [Google Generative AI](https://www.home-assistant.io/integrations/google_generative_ai_conversation) | Conversation agents |
| [Apple TV](https://www.home-assistant.io/integrations/apple_tv) | Core integration |
| [Bermuda BLE Trilateration](https://github.com/agittins/bermuda) | HACS custom integration |
| [Bluetooth](https://www.home-assistant.io/integrations/bluetooth) / [iBeacon](https://www.home-assistant.io/integrations/ibeacon) | Core integration |
| [CO2 Signal](https://www.home-assistant.io/integrations/co2signal) | Core integration |
| [EdgeOS](https://github.com/elad-bar/ha-edgeos) | HACS custom integration |
| [Generic Camera](https://www.home-assistant.io/integrations/generic) | Core integration |
| [go2rtc](https://www.home-assistant.io/integrations/go2rtc) | Core integration |
| [HACS](https://hacs.xyz) | Custom integration manager |
| [HASS.Agent](https://github.com/hass-agent/HASS.Agent-Integration) | HACS custom integration |
| [Husqvarna Automower BLE](https://www.home-assistant.io/integrations/husqvarna_automower_ble) | Core integration |
| [Google Sheets](https://www.home-assistant.io/integrations/google_sheets) / [Google Translate](https://www.home-assistant.io/integrations/google_translate) | Core integration |
| [LLM Vision](https://github.com/valentinfrlch/ha-llmvision) | HACS custom integration |
| [LocalTuya](https://github.com/rospogrigio/localtuya) | HACS custom integration |
| [Magic Areas](https://github.com/jseidl/magic-areas) | HACS custom integration |
| [Matter](https://www.home-assistant.io/integrations/matter) | Core integration |
| [Music Assistant](https://www.home-assistant.io/integrations/music_assistant) | Core integration |
| [Network Scanner](https://github.com/parvez/network_scanner) | HACS custom integration |
| [Node-RED Companion](https://github.com/zachowj/hass-node-red) | HACS custom integration |
| [OpenAI TTS](https://github.com/sfortis/openai_tts) | HACS custom integration |
| [OpenClaw](https://github.com/techartdev/OpenClawHomeAssistantIntegration) | HACS custom integration |
| [Passive BLE Monitor](https://github.com/custom-components/ble_monitor) | HACS custom integration |
| [Reolink](https://www.home-assistant.io/integrations/reolink) | Core integration |
| [RuuviTag BLE](https://www.home-assistant.io/integrations/ruuvitag_ble) | Core integration |
| [Shelly](https://www.home-assistant.io/integrations/shelly) | Core integration |
| [Shopping List](https://www.home-assistant.io/integrations/shopping_list) | Core integration |
| [Simple Inventory](https://github.com/blaineventurine/simple_inventory) | HACS custom integration |
| [Sonoff (LAN)](https://github.com/AlexxIT/SonoffLAN) | HACS custom integration |
| [Spook](https://github.com/frenck/spook) | HACS custom integration |
| [Switch as X](https://www.home-assistant.io/integrations/switch_as_x) | Core integration |
| [Synology DSM](https://www.home-assistant.io/integrations/synology_dsm) | Core integration |
| [TP-Link Kasa](https://www.home-assistant.io/integrations/tplink) | Core integration |
| [Tuya](https://www.home-assistant.io/integrations/tuya) | Core integration |
| [Tuya Local](https://github.com/make-all/tuya-local) | HACS custom integration |
| [Uptime Kuma](https://www.home-assistant.io/integrations/uptime_kuma) | Core integration |
| [Victron BLE](https://www.home-assistant.io/integrations/victron_ble) | Core integration |
| [Watchman](https://github.com/dummylabs/thewatchman) | HACS custom integration |
| [Wyoming](https://www.home-assistant.io/integrations/wyoming) | Core integration |
| [Xiaomi BLE](https://www.home-assistant.io/integrations/xiaomi_ble) | Core integration |
| [Yale](https://www.home-assistant.io/integrations/yale) / [Yale BLE](https://www.home-assistant.io/integrations/yalexs_ble) | Core integration |

</details>

### HACS — custom integrations (50 installed)

| Integration | Repo |
|---|---|
| AC Infinity | [dalinicus/homeassistant-acinfinity](https://github.com/dalinicus/homeassistant-acinfinity) |
| AC Infinity (AI+ Optimized) | [JoshuaSeidel/homeassistant-acinfinity](https://github.com/JoshuaSeidel/homeassistant-acinfinity) |
| Alarmo | [nielsfaber/alarmo](https://github.com/nielsfaber/alarmo) |
| Alexa Media Player | [alandtse/alexa_media_player](https://github.com/alandtse/alexa_media_player) |
| Auto Arm | [rhizomatics/autoarm](https://github.com/rhizomatics/autoarm) |
| Average Sensor | [Limych/ha-average](https://github.com/Limych/ha-average) |
| Bermuda BLE Trilateration | [agittins/bermuda](https://github.com/agittins/bermuda) |
| Beszel API | [Ronjar/beszel-ha](https://github.com/Ronjar/beszel-ha) |
| Better Thermostat | [KartoffelToby/better_thermostat](https://github.com/KartoffelToby/better_thermostat) |
| Browser Mod | [thomasloven/hass-browser_mod](https://github.com/thomasloven/hass-browser_mod) |
| Chore Helper | [VolantisDev/ha-chore-helper](https://github.com/VolantisDev/ha-chore-helper) |
| Crop Steering System | [JakeTheRabbit/HA-Irrigation-Strategy](https://github.com/JakeTheRabbit/HA-Irrigation-Strategy) |
| Daily Sensor | [jeroenterheerdt/HADailySensor](https://github.com/jeroenterheerdt/HADailySensor) |
| Device Pulse | [studiobts/home-assistant-device-pulse](https://github.com/studiobts/home-assistant-device-pulse) |
| Dual Smart Thermostat | [swingerman/ha-dual-smart-thermostat](https://github.com/swingerman/ha-dual-smart-thermostat) |
| Favicon Changer | [thomasloven/hass-favicon](https://github.com/thomasloven/hass-favicon) |
| Frigate | [blakeblackshear/frigate-hass-integration](https://github.com/blakeblackshear/frigate-hass-integration) |
| HACS | [hacs/integration](https://github.com/hacs/integration) |
| HASS.Agent | [hass-agent/HASS.Agent-Integration](https://github.com/hass-agent/HASS.Agent-Integration) |
| HVAC Group | [tetele/hvac_group](https://github.com/tetele/hvac_group) |
| IntesisHome | [jnimmo/hass-intesishome](https://github.com/jnimmo/hass-intesishome) |
| LLM Vision | [valentinfrlch/ha-llmvision](https://github.com/valentinfrlch/ha-llmvision) |
| LocalTuya | [rospogrigio/localtuya](https://github.com/rospogrigio/localtuya) |
| Magic Areas | [jseidl/magic-areas](https://github.com/jseidl/magic-areas) |
| MeasureIt | [danieldotnl/ha-measureit](https://github.com/danieldotnl/ha-measureit) |
| MetService New Zealand Weather | [ciejer/metservice-weather](https://github.com/ciejer/metservice-weather) |
| Mold Risk Index | [Strixx76/mold_risk_index](https://github.com/Strixx76/mold_risk_index) |
| Network Scanner | [parvez/network_scanner](https://github.com/parvez/network_scanner) |
| Node-RED Companion | [zachowj/hass-node-red](https://github.com/zachowj/hass-node-red) |
| ntfy.sh Notifications | [ivanmihov/homeassistant-ntfy.sh](https://github.com/ivanmihov/homeassistant-ntfy.sh) |
| OpenAI GPT-4o Mini TTS | [wifiuk/OpenAI-GPT-4o-Mini-TTS-Home-Assistant-Integration](https://github.com/wifiuk/OpenAI-GPT-4o-Mini-TTS-Home-Assistant-Integration) |
| OpenAI TTS | [sfortis/openai_tts](https://github.com/sfortis/openai_tts) |
| OpenAI Whisper API | [einToast/openai_stt_ha](https://github.com/einToast/openai_stt_ha) |
| OpenClaw | [techartdev/OpenClawHomeAssistantIntegration](https://github.com/techartdev/OpenClawHomeAssistantIntegration) |
| Passive BLE Monitor | [custom-components/ble_monitor](https://github.com/custom-components/ble_monitor) |
| PID Controller | [soloam/ha-pid-controller](https://github.com/soloam/ha-pid-controller) |
| Powercalc | [bramstroker/homeassistant-powercalc](https://github.com/bramstroker/homeassistant-powercalc) |
| Pyscript Python Scripting | [custom-components/pyscript](https://github.com/custom-components/pyscript) |
| Scheduler Component | [nielsfaber/scheduler-component](https://github.com/nielsfaber/scheduler-component) |
| Simple Inventory | [blaineventurine/simple_inventory](https://github.com/blaineventurine/simple_inventory) |
| Sonoff (LAN) | [AlexxIT/SonoffLAN](https://github.com/AlexxIT/SonoffLAN) |
| Spook | [frenck/spook](https://github.com/frenck/spook) |
| Spotify Voice Assistant Search | [cauld/spotify-voice-assistant](https://github.com/cauld/spotify-voice-assistant) |
| Supernotify | [rhizomatics/supernotify](https://github.com/rhizomatics/supernotify) |
| Tuya Local | [make-all/tuya-local](https://github.com/make-all/tuya-local) |
| Ubiquiti EdgeOS Routers | [elad-bar/ha-edgeos](https://github.com/elad-bar/ha-edgeos) |
| Variable | [snarky-snark/home-assistant-variables](https://github.com/snarky-snark/home-assistant-variables) |
| Watchman | [dummylabs/thewatchman](https://github.com/dummylabs/thewatchman) |
| xAI Conversation | [pajeronda/xai_conversation](https://github.com/pajeronda/xai_conversation) |
| xAI Grok Conversation | [braytonstafford/grok_conversation](https://github.com/braytonstafford/grok_conversation) |

### HACS — frontend cards (43 installed)

| Card | Repo |
|---|---|
| Advanced Camera Card | [dermotduffy/advanced-camera-card](https://github.com/dermotduffy/advanced-camera-card) |
| Alarmo Card | [nielsfaber/alarmo-card](https://github.com/nielsfaber/alarmo-card) |
| Anchor Card | [ShadowAya/anchor-card](https://github.com/ShadowAya/anchor-card) |
| Bar Card | [custom-cards/bar-card](https://github.com/custom-cards/bar-card) |
| Battery State Card | [maxwroc/battery-state-card](https://github.com/maxwroc/battery-state-card) |
| Bubble Card | [Clooos/Bubble-Card](https://github.com/Clooos/Bubble-Card) |
| Button Card | [custom-cards/button-card](https://github.com/custom-cards/button-card) |
| Card-mod | [thomasloven/lovelace-card-mod](https://github.com/thomasloven/lovelace-card-mod) |
| Custom Card Features | [Nerwyn/custom-card-features](https://github.com/Nerwyn/custom-card-features) |
| Custom Icon Color | [Mariusthvdb/custom-icon-color](https://github.com/Mariusthvdb/custom-icon-color) |
| Elapsed Time Card | [Kirbo/ha-lovelace-elapsed-time-card](https://github.com/Kirbo/ha-lovelace-elapsed-time-card) |
| Energy Overview Card | [Sese-Schneider/ha-energy-overview-card](https://github.com/Sese-Schneider/ha-energy-overview-card) |
| Floorplan | [ExperienceLovelace/ha-floorplan](https://github.com/ExperienceLovelace/ha-floorplan) |
| Fold Entity Row | [thomasloven/lovelace-fold-entity-row](https://github.com/thomasloven/lovelace-fold-entity-row) |
| Horizon Card | [rejuvenate/lovelace-horizon-card](https://github.com/rejuvenate/lovelace-horizon-card) |
| Layout Card | [thomasloven/lovelace-layout-card](https://github.com/thomasloven/lovelace-layout-card) |
| LLM Vision Card | [valentinfrlch/llmvision-card](https://github.com/valentinfrlch/llmvision-card) |
| Lovelace Badge Card | [thomasloven/lovelace-badge-card](https://github.com/thomasloven/lovelace-badge-card) |
| Lovelace Canary | [jcwillox/lovelace-canary](https://github.com/jcwillox/lovelace-canary) |
| Mini-Climate Card | [artem-sedykh/mini-climate-card](https://github.com/artem-sedykh/mini-climate-card) |
| Mini-Graph Card | [kalkih/mini-graph-card](https://github.com/kalkih/mini-graph-card) |
| Modern Circular Gauge | [selvalt7/modern-circular-gauge](https://github.com/selvalt7/modern-circular-gauge) |
| More Info Card | [thomasloven/lovelace-more-info-card](https://github.com/thomasloven/lovelace-more-info-card) |
| Multiple Entity Row | [benct/lovelace-multiple-entity-row](https://github.com/benct/lovelace-multiple-entity-row) |
| Mushroom | [piitaya/lovelace-mushroom](https://github.com/piitaya/lovelace-mushroom) |
| Navbar Card | [joseluis9595/lovelace-navbar-card](https://github.com/joseluis9595/lovelace-navbar-card) |
| Numberbox Card | [junkfix/numberbox-card](https://github.com/junkfix/numberbox-card) |
| Paper Buttons Row | [jcwillox/lovelace-paper-buttons-row](https://github.com/jcwillox/lovelace-paper-buttons-row) |
| Plotly Graph Card | [dbuezas/lovelace-plotly-graph-card](https://github.com/dbuezas/lovelace-plotly-graph-card) |
| Psychrometric Chart Advanced | [guiohm79/psychrometric-chart-advanced](https://github.com/guiohm79/psychrometric-chart-advanced) |
| Scheduler Card | [nielsfaber/scheduler-card](https://github.com/nielsfaber/scheduler-card) |
| Slider Entity Row | [thomasloven/lovelace-slider-entity-row](https://github.com/thomasloven/lovelace-slider-entity-row) |
| Stack-in-Card | [custom-cards/stack-in-card](https://github.com/custom-cards/stack-in-card) |
| TDV Bar | [tdvtdv/ha-tdv-bar](https://github.com/tdvtdv/ha-tdv-bar) |
| Template Entity Row | [thomasloven/lovelace-template-entity-row](https://github.com/thomasloven/lovelace-template-entity-row) |
| Text Divider Row | [iantrich/text-divider-row](https://github.com/iantrich/text-divider-row) |
| TimeFlow Card | [Rishi8078/TimeFlow-Card](https://github.com/Rishi8078/TimeFlow-Card) |
| Timer Bar Card | [rianadon/timer-bar-card](https://github.com/rianadon/timer-bar-card) |
| Timeline Card | [weedpump/timeline-card](https://github.com/weedpump/timeline-card) |
| Vertical Stack-in-Card | [ofekashery/vertical-stack-in-card](https://github.com/ofekashery/vertical-stack-in-card) |
| Weather Chart Card | [mlamberts78/weather-chart-card](https://github.com/mlamberts78/weather-chart-card) |
| Week Planner Card | [FamousWolf/week-planner-card](https://github.com/FamousWolf/week-planner-card) |
| AC Infinity Lovelace Card | [JoshuaSeidel/hass-acinfinity-lovelace-card](https://github.com/JoshuaSeidel/hass-acinfinity-lovelace-card) |

### HACS — template helpers (3 installed)

- [Petro31/easy-time-jinja](https://github.com/Petro31/easy-time-jinja) — Jinja-friendly time formatting.
- [SirGoodenough/Availability-Template](https://github.com/SirGoodenough/Availability-Template) — reusable `availability` template for template entities.
- [TheFes/relative-time-plus](https://github.com/TheFes/relative-time-plus) — richer relative-time strings.

---

## External resources

### Crop steering and irrigation

- [Killerherts/nodeRed-HA-GrowingFunctions](https://github.com/Killerherts/nodeRed-HA-GrowingFunctions) — Node-RED flows for HA grow automation.
- [jeemers/Homegrown-Assistant](https://github.com/jeemers/Homegrown-Assistant) — A great HA + ESPHome crop-steering reference.
- [Intergalactic-XYZ/awesome-cropsteering](https://github.com/Intergalactic-XYZ/awesome-cropsteering) — Curated awesome-list.
- [cropsteering/OS-SDI12](https://github.com/cropsteering/OS-SDI12) — Raw SDI-12 to MQTT, no Home Assistant required.

### Sensors and firmware

- [Chill-Division/M5Stack-ESPHome](https://github.com/Chill-Division/M5Stack-ESPHome) — M5Stack ESPHome configurations for indoor gardening.
- [Chill-Division/sdi12-substrate-sensor](https://github.com/Chill-Division/sdi12-substrate-sensor) — Calibrated SDI-12 substrate sensor with MQTT auto-discovery. Use this rather than rolling your own TEROS-12.
- [kromadg/soil-sensor](https://github.com/kromadg/soil-sensor) — THC-S soil-sensor reference.
- [M5Stack sensors](https://shop.m5stack.com/collections/m5-sensor) — most of the hardware in this repo.

### Nutrient calculators

- [Hydrobuddy](https://scienceinhydroponics.com/2016/03/the-first-free-hydroponic-nutrient-calculator-program-o.html) — open-source hydroponic nutrient calculator.
- [Open Salts](http://opensalts.wikidot.com/) — ingredient breakdowns of common commercial nutrients.

### Growing guides

- [Grodan Cannabis Edition 2024 (PDF)](https://www.grodan101.com/siteassets/downloads/downloads-na-101/grow-guide-2023/grow-guide---cannabis-edition-2024.pdf)
- [Athena Pro Line Handbook (issuu)](https://issuu.com/athenaag/docs/athena_hb_me)
- [Growlink Crop Steering primer](https://www.growlink.ag/crop-steering)
- [Ultimate Grow Room HVAC Guide (PDF)](https://midwestmachinery.net/wp-content/uploads/2020/01/Ultimate-Grow-Room-HVAC-Guide.pdf)
- [Grow-room airflow CFD walkthrough (YouTube)](https://youtu.be/TOYe9ZFVRy8?si=7zaU3VCEeO92pmpk&t=11)
- [How to splice sensor wires properly (YouTube)](https://www.youtube.com/watch?v=aTpYi5nYjO0)

### Adjacent open-source projects

- [OpenTHC](https://github.com/openthc) — open cannabis-industry standards and tooling.
- [farmOS](https://github.com/farmOS/farmOS) — open-source farm record-keeping.
- [OpenFOAM](https://www.openfoam.com/) — CFD modelling, useful for airflow simulation.

---

## Screenshots

<a href="https://github.com/JakeTheRabbit/HAGR/assets/123831499/673f8f7f-dcf0-4fb1-9b35-9f19ce383b5e" target="_blank">
    <img src="https://github.com/JakeTheRabbit/HAGR/assets/123831499/673f8f7f-dcf0-4fb1-9b35-9f19ce383b5e" alt="Dashboard overview" width="200">
</a>
<a href="https://github.com/JakeTheRabbit/HAGR/assets/123831499/847fe601-8523-45e5-9813-8af4eafff33d" target="_blank">
    <img src="https://github.com/JakeTheRabbit/HAGR/assets/123831499/847fe601-8523-45e5-9813-8af4eafff33d" alt="Environment config" width="200">
</a>
<a href="https://github.com/JakeTheRabbit/HAGR/assets/123831499/f788d65a-31c6-459c-a08a-5912b7a1fba6" target="_blank">
    <img src="https://github.com/JakeTheRabbit/HAGR/assets/123831499/f788d65a-31c6-459c-a08a-5912b7a1fba6" alt="Crop steering config" width="200">
</a>

Peri-pump automatic batch-tank filling:

<img width="1205" alt="Batch tank filling" src="https://github.com/user-attachments/assets/344c5632-e1da-451d-a06a-b8f062dcf741" />

---

## A note on secrets

The ESPHome configs under [`esphome/`](esphome/) still have device-specific credentials inline in places — API encryption keys, OTA passwords, WiFi AP fallback passwords. Before flashing anything in production, swap those for [`!secret` references](https://esphome.io/guides/faq.html#how-do-i-use-secrets-passwords-in-yaml) backed by a per-device `esphome/secrets.yaml`. Start from [`esphome/secrets.yaml.example`](esphome/secrets.yaml.example) and never commit the real `secrets.yaml`.

If you're forking this for your own build, rotate any credential you inherit from a committed example.

---

Still a work in progress. [Issues](https://github.com/JakeTheRabbit/HAGR/issues) and [pull requests](https://github.com/JakeTheRabbit/HAGR/pulls) are welcome.

[MIT licensed](LICENSE).