# Blueprints

Home Assistant blueprints — import once, then point them at your entities from the UI.

## Importing

1. Open the blueprint YAML in this directory and grab its raw GitHub URL.
2. In Home Assistant: **Settings → Automations & Scenes → Blueprints → Import Blueprint**.
3. Paste the URL and import.
4. Click **Create automation** from the imported blueprint and fill in the inputs.

For local development you can also drop the file into `<config>/blueprints/automation/<your-namespace>/` and HA will pick it up on reload.

## Files

| Blueprint | What it does | Key inputs |
|---|---|---|
| [`co2_control_and_alerts.yaml`](co2_control_and_alerts.yaml) | Day/night CO2 setpoints, hysteresis switching, relay-stuck safety auto-off, low and high alerts with cooldowns, optional light auto-dim on sustained low CO2. | CO2 sensors (primary + optional secondary), target inputs, CO2 switch, light group, day/night times, alert toggles. |
| [`grow_room_env_threshold_alerts.yaml`](grow_room_env_threshold_alerts.yaml) | Consolidated environmental threshold alerts. Separate day/night helpers per sensor, optional persistence delay, cooldown between notifications, pause switch, multi-device notification targets via an action input. | Room name, alerts-paused boolean, day/night schedule, per-sensor day/night thresholds, notify-target action. |
| [`auto_temp_triggered_light_dimming.yaml`](auto_temp_triggered_light_dimming.yaml) | When the room overheats, dim the lights and keep dimming until temp normalises. Plant-safe heat protection. | Temperature sensor, threshold (input_number), light group. |
| [`light_leak_detection.yaml`](light_leak_detection.yaml) | Critical light-leak alarm. Any illuminance sensor above the threshold while your lights-on binary sensor is off forces the grow lights off and runs your notification action. The single highest-stakes automation in a flowering room. | Ambient light sensors (multi), lux threshold, lights-on binary sensor, grow-light group, notification action. |

## See also

- [`../addons/appdaemon/grow_monitor/`](../addons/appdaemon/grow_monitor/) — equivalent consolidated alerting via AppDaemon, with severity grading and an optional AI-generated summary line.
- [`../automations/`](../automations/) — raw YAML automations (no blueprint indirection).
