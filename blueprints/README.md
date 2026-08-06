# Blueprints

Home Assistant blueprints — import once, then point them at your entities from the UI.

## Importing

1. Open the blueprint YAML in this directory and grab its raw GitHub URL.
2. In Home Assistant: **Settings → Automations & Scenes → Blueprints → Import Blueprint**.
3. Paste the URL and import.
4. Click **Create automation** from the imported blueprint and fill in the inputs.

For local development you can also drop the file into `<config>/blueprints/automation/<your-namespace>/` and HA will pick it up on reload.

## Files

All five target **Home Assistant 2024.10+** (they use the `triggers:`/`actions:` syntax, `tts.speak`, `notify.send_message`, and scene snapshots). Three of them need extra helpers — see **Required helpers** below.

| Blueprint | What it does | Key inputs |
|---|---|---|
| [`co2_control_and_alerts.yaml`](co2_control_and_alerts.yaml) | Day/night CO2 setpoints, hysteresis switching, high-CO2 auto-off (primary + optional backup sensor, with a control lockout so it won't re-open while high), relay-stuck/empty-tank auto-off, sustained-low alerts with cooldown, optional light auto-dim that restores on recovery. | CO2 sensors (primary + optional secondary), target inputs, CO2 switch, light group, day/night times, max valve runtime, alert toggles, last-alert helpers. |
| [`grow_room_env_threshold_alerts.yaml`](grow_room_env_threshold_alerts.yaml) | Consolidated environmental threshold alerts. Separate day/night helpers per sensor, optional persistence delay, helper-backed cooldown between notifications, pause switch, multi-device notification targets via an action input. | Room name, alerts-paused boolean, day/night schedule, per-sensor day/night thresholds, notify-target action, last-notified helper. |
| [`auto_temp_triggered_light_dimming.yaml`](auto_temp_triggered_light_dimming.yaml) | When the room overheats, step-dim the lights until temp normalises, then restore them to their pre-dim brightness. Only dims lights that are already on, with a minimum-brightness floor. Plant-safe heat protection. | Temperature sensor, threshold (input_number), light group, dim step, min brightness, check interval, recovery time. |
| [`light_leak_detection.yaml`](light_leak_detection.yaml) | Critical light-leak alarm. Any illuminance sensor above the threshold while your lights-on binary sensor is off forces the grow lights off and runs your notification action — re-checked on restart and every minute. The single highest-stakes automation in a flowering room. | Ambient light sensors (multi), lux threshold, lights-on binary sensor, grow-light group, notification action. |
| [`tank_warmup_circulation.yaml`](tank_warmup_circulation.yaml) | Warms a batch/reservoir tank by circulating water through a return valve until it hits a target minimum temperature, then stops with a staged shutdown (pump first, valve after — never deadheads). Yields instantly to irrigation/dosing: closes the valve and hands the pumps over the moment any "interruptor" entity fires, then resumes once watering finishes if the tank is still cold. Dry-run protection, dead-sensor stop, max-runtime cap, restart hysteresis. | Tank temp sensor, target + hysteresis, return valve, pump switch(es), master + active toggle helpers, irrigation interruptors (multi), optional tank-level sensor, max runtime, optional start/stop actions. |

## Required helpers

A couple of these store timestamps in `input_datetime` helpers so cooldowns survive restarts. Create them under **Settings → Devices & Services → Helpers** (or in YAML) before importing:

- **`co2_control_and_alerts.yaml`** — the `input_number` and `input_datetime` helpers listed in the blueprint's own description (targets, tolerances, on/off times, alert windows, and `last_low_co2_alert` / `last_high_co2_alert`).
- **`grow_room_env_threshold_alerts.yaml`** — one `input_datetime` (date + time) for the notification cooldown, e.g. `input_datetime.f1_env_last_alert`, plus the per-sensor day/night threshold `input_number`s.
- **`tank_warmup_circulation.yaml`** — two toggle (`input_boolean`) helpers: a master enable switch and an internal "active" flag the blueprint owns.

The other two need no extra helpers.

## See also

- [`../addons/appdaemon/grow_monitor/`](../addons/appdaemon/grow_monitor/) — equivalent consolidated alerting via AppDaemon, with severity grading and an optional AI-generated summary line.
- [`../automations/`](../automations/) — raw YAML automations (no blueprint indirection).
