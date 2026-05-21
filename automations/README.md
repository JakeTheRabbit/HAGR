# Automations

Ready-to-import Home Assistant automations and the explainers that go with them.

Drop the YAMLs into your `packages/` directory or paste them into the Automations editor. Swap the prefixed entity IDs (`f1_*`, `aqua_pro_*`, `atlas_hydro_1_*`, etc.) for the ones in your setup.

## Safety and alerting

| File | What it does |
|---|---|
| [`light_leak_detection.yaml`](light_leak_detection.yaml) | The single most important one. Any ambient-light sensor above 1 lux during the dark period kills the grow lights and fires a critical notification. Also packaged as a [blueprint](../blueprints/light_leak_detection.yaml). |
| [`co2_multi_sensor_shutoff.yaml`](co2_multi_sensor_shutoff.yaml) | Closes the CO2 solenoid if any of N room sensors reads above 1800 ppm. Catches stratified pockets that single-sensor shutoff misses. |
| [`sensor_unavailability_watchdog.yaml`](sensor_unavailability_watchdog.yaml) | Alerts when temp, RH, or CO2 sensors have been offline for 10 min. Fail-safe closes the CO2 solenoid if the CO2 sensor itself dies. |
| [`daily_safe_state_audit.yaml`](daily_safe_state_audit.yaml) | At 03:00 every night, force-closes every valve, the CO2 solenoid, and the humidifier. Belt-and-braces sweep for stuck valves and forgotten overrides. |
| [`CO2_low_Alert.yaml`](CO2_low_Alert.yaml) | Actionable Android alert when CO2 drops 300 ppm below target for 5+ min, or the CO2 valve has been open >5 min (likely empty tank). Bypasses do-not-disturb. |

## Lighting and environment

| File | What it does |
|---|---|
| [`light_acclimation.yaml`](light_acclimation.yaml) | Ramps brightness from 50% to 90% over 21 days then turns itself off. Triggered at lights-on. |
| [`humidifier_off_at_lights_off.yaml`](humidifier_off_at_lights_off.yaml) | Hard-pins the humidifier off the moment lights go off. Lets the dehumidifiers run the dark period uncontested. |
| [`Adjust ACI Fan Speed External Sensor.md`](Adjust%20ACI%20Fan%20Speed%20External%20Sensor.md) | (Explainer) AC Infinity fan speed (0-10) from an external temp sensor with day/night targets. Helpers + trigger logic. |
| [`Leaf_VPD.md`](Leaf_VPD.md) | (Explainer) Leaf-VPD vs air-VPD, and the control system that uses an IR leaf-temp sensor to drive a humidifier or dehumidifier. |

## Irrigation and dosing

| File | What it does |
|---|---|
| [`refill_tank_triggered_by_low_level.yaml`](refill_tank_triggered_by_low_level.yaml) | Full batch-tank refill + Athena Pro Line dose sequence. Fires on low water level, manual button, or tank-empty signal. Pre-fill → refill to volume → dose A/B/C/Bloom/Core → mix delay → pH/EC verification → completion alert. |
| [`Automatic_batch_tank_filling_full_explainer.md`](Automatic_batch_tank_filling_full_explainer.md) | (Explainer) Long-form walk-through of the refill and multi-part Athena dosing logic. Read this alongside the YAML. |
| [`auto dosing athena.md`](auto%20dosing%20athena.md) | (Explainer) Helper definitions (EC target, solution volume) and the dosing-node sequence for an Athena auto-dose workflow. |
| [`crop steering node red flow`](crop%20steering%20node%20red%20flow) | Older Node-RED export of a crop-steering implementation. Most people should use the YAML package in [`../Packages/CropSteering/`](../Packages/CropSteering/) instead. |

## Access control

| File | What it does |
|---|---|
| [`door_open_siren.yaml`](door_open_siren.yaml) | Sounds a siren and notifies operators if a critical door is left open >20 s. Cancels on close. One instance per door. |
| [`auto_door_lock.yaml`](auto_door_lock.yaml) | Locks the door 20 s after it closes. Kills the "I forgot to lock it" failure mode. |

## CO2 blueprint imports

| File | What it does |
|---|---|
| [`co2_automation.MD`](co2_automation.MD) | (Explainer) Import link and notes for the CO2 control blueprint. See also [`../blueprints/co2_control_and_alerts.yaml`](../blueprints/co2_control_and_alerts.yaml). |

## Conventions

- **Room prefix.** Examples use `f1_` for "room 1". Substitute your own room prefix.
- **Notification target.** Replace `notify.notify` or `notify.mobile_app_*` with your real notify service.
- **Pumps and switches.** Replace `switch.cs_irrigation_pump`, `switch.f1_co2`, `switch.YOUR_*`, etc. with the actual entity IDs of your hardware.
- **Helpers.** A lot of these reference `input_number`, `input_datetime`, and `input_boolean` helpers. Create them in **Settings → Devices & Services → Helpers** before importing the automation, or pull them in via a package file.

## See also

- [`../blueprints/`](../blueprints/) — the same patterns packaged as importable blueprints (configurable from the UI).
- [`../Packages/CropSteering/`](../Packages/CropSteering/) — the full crop-steering package.
- [`../addons/appdaemon/grow_monitor/`](../addons/appdaemon/grow_monitor/) — consolidated environmental alerting via AppDaemon, as an alternative to per-sensor automations.
