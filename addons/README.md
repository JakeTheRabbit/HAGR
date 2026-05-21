# Add-ons

AppDaemon-based apps that supplement the standard Home Assistant automations.

| Add-on | What it does |
|---|---|
| [`appdaemon/grow_monitor/`](appdaemon/grow_monitor/) | **Grow Room Monitor** — consolidated environmental alerting with day/night-aware thresholds, trend analysis, severity grading, mute and pause actions, and an optional OpenAI-generated situation summary in the alert body. See [`Readme.MD`](appdaemon/grow_monitor/Readme.MD) for full setup. |

## Why AppDaemon?

The patterns in this app — multi-sensor consolidation, persistent mute state across restarts, trend evaluation, AI summarisation — are awkward in pure HA YAML and natural in Python. Install the AppDaemon HA add-on, drop the app into `/config/appdaemon/apps/`, register it in `apps.yaml`.

## See also

- [`../blueprints/grow_room_env_threshold_alerts.yaml`](../blueprints/grow_room_env_threshold_alerts.yaml) — simpler pure-YAML alternative if you don't want to run AppDaemon.
