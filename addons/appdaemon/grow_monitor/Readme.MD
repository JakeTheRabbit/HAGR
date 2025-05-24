# 🤖 Grow Room AI Monitor v3.1 for Home Assistant

An intelligent AppDaemon-based monitoring system for cannabis grow rooms with AI-powered **situation summaries**, smart error detection, configurable alert muting, and spam-free notifications.

## 🚀 Feature Summary

### 🌱 Advanced Environmental Monitoring
- **7 Sensor Types**: Temperature, Humidity, CO₂, VPD, Leaf VPD, VWC (Volumetric Water Content), pwEC (Pore Water Electrical Conductivity).
- **Day/Night Awareness**: Different thresholds for lights-on / lights-off periods.
- **Trend Analysis**: Calculates sensor trends (RAPID_RISE, STABLE, etc.) to inform alert severity.
- **Centralized Entity Configuration**: All Home Assistant entities easily defined in `grow_monitor.py`.

### 🧠 AI-Powered Intelligence
- **OpenAI Integration**: Provides concise AI-generated summaries of current environmental issues in notifications (describes *what* is wrong, not how to fix it).
- **Periodic AI Trend Analysis**: A framework for future enhancements to analyze overall trends (currently logs a placeholder).

### 🚨 Intelligent Alert System
- **Smart Summary Notifications**: Consolidates multiple active alerts into a single notification. Notifications use a consistent tag to replace previous ones.
- **Configurable Severity Levels**: IGNORE, NORMAL, URGENT, CRITICAL, based on violation duration and trend.
- **Intelligent Rate Limiting**:
    - Base 1-hour cooldown between summary notifications.
    - Extends to a 6-hour quiet period after any sensor alert is acknowledged (muted) via a notification action.
    - New *critical* alerts bypass cooldowns for immediate notification.
- **Actionable Notifications**:
    - Buttons to mute specific alerting sensors for 1 hour or 6 hours.
    - Button to activate a global "Pause ALL Alerts" `input_boolean`.
- **Per-Sensor Muting:** Temporarily silence alerts for specific sensors if an issue is known or non-critical.
- **Persistent State**: Remembers user-muted sensors and notification cooldown states across AppDaemon restarts (via a `.pkl` data file).

### 💾 Architecture
- **Python State Management**: Core logic and alert states managed within the Python script.
- **Rolling Data Buffers**: Maintains recent sensor history for trend calculation.
- **Event-Driven & Scheduled Tasks**: Responds to sensor updates and performs periodic checks.

## 📋 Prerequisites

- Home Assistant with the AppDaemon add-on.
- Environmental sensor entities configured in Home Assistant for all monitored parameters.
- `input_datetime` entities for lights on/off schedule.
- `input_number` entities for all alert thresholds.
- `input_boolean` entity for globally pausing alerts.
- A `notify` service for mobile app notifications (e.g., `notify.mobile_app_yourphone`).
- (Optional) OpenAI API Key for AI-generated notification summaries.
- A Home Assistant Script (`script.grow_monitor_mute_sensor`) for handling notification mute actions.

## 🛠️ Installation & Configuration

### 1. File Placement
   - **`grow_monitor.py` (v3.1 script):** Place in your active AppDaemon apps directory. Based on current logs for many HAOS/Supervised setups, this is likely:
     `\\YOUR_HA_IP\config\apps\grow_monitor.py`
     (or `/config/apps/grow_monitor.py` from the perspective of the AppDaemon container).
   - **`apps.yaml`:** Place in the same directory as `grow_monitor.py` (e.g., `\\YOUR_HA_IP\config\apps\apps.yaml`).
   - **`appdaemon.yaml` (Main Configuration):** Ensure the `appdaemon.yaml` file that your AppDaemon instance reads (likely `\\YOUR_HA_IP\config\appdaemon.yaml`) contains:
     ```yaml
     appdaemon:
       # ... your latitude, longitude, elevation ...
       time_zone: Your/Timezone # e.g., Pacific/Auckland
       app_dir: /config/apps 
       plugins:
         HASS:
           type: hass
     ```

### 2. Configure `apps.yaml`
   ```yaml
   # Example: \\YOUR_HA_IP\config\apps\apps.yaml
   grow_monitor:
     module: grow_monitor
     class: GrowRoomAIMonitor
     openai_api_key: "sk-YOUR_OPENAI_API_KEY_HERE" # Or use !secret```
3. Create Home Assistant Helper Entities (YAML)Copy and paste the following block into your Home Assistant configuration.yaml file (or a separate YAML file that you !include, like input_helpers.yaml). Then, restart Home Assistant (or Reload Input Helpers from Developer Tools).Replace YOUR_PREFIX_ with your desired prefix (e.g., f1_, grow_room_) and update the ENTITIES dictionary in grow_monitor.py accordingly.# In your configuration.yaml or a dedicated !include file for inputs
```
input_datetime:
  YOUR_PREFIX_lights_on_time:
    name: "YOUR_PREFIX Lights On Time"
    has_date: false
    has_time: true
  YOUR_PREFIX_lights_off_time:
    name: "YOUR_PREFIX Lights Off Time"
    has_date: false
    has_time: true

input_boolean:
  YOUR_PREFIX_environmental_alerts_paused:
    name: "YOUR_PREFIX Environmental Alerts Paused"
    icon: mdi:pause-circle-outline

input_number:
  # Temperature Thresholds (°C)
  YOUR_PREFIX_day_temp_high_alert:
    name: "YOUR_PREFIX Day Temp High Alert"
    min: 15
    max: 40
    step: 0.1
    unit_of_measurement: "°C"
    icon: mdi:thermometer-chevron-up
  YOUR_PREFIX_day_temp_low_alert:
    name: "YOUR_PREFIX Day Temp Low Alert"
    min: 10
    max: 30
    step: 0.1
    unit_of_measurement: "°C"
    icon: mdi:thermometer-chevron-down
  YOUR_PREFIX_night_temp_high_alert:
    name: "YOUR_PREFIX Night Temp High Alert"
    min: 10
    max: 35
    step: 0.1
    unit_of_measurement: "°C"
    icon: mdi:thermometer-chevron-up
  YOUR_PREFIX_night_temp_low_alert:
    name: "YOUR_PREFIX Night Temp Low Alert"
    min: 5
    max: 25
    step: 0.1
    unit_of_measurement: "°C"
    icon: mdi:thermometer-chevron-down

  # Humidity Thresholds (%)
  YOUR_PREFIX_day_humidity_high_alert:
    name: "YOUR_PREFIX Day Humidity High Alert"
    min: 30
    max: 90
    step: 1
    unit_of_measurement: "%"
    icon: mdi:water-percent-alert
  YOUR_PREFIX_day_humidity_low_alert:
    name: "YOUR_PREFIX Day Humidity Low Alert"
    min: 20
    max: 80
    step: 1
    unit_of_measurement: "%"
    icon: mdi:water-percent-alert
  YOUR_PREFIX_night_humidity_high_alert:
    name: "YOUR_PREFIX Night Humidity High Alert"
    min: 30
    max: 95
    step: 1
    unit_of_measurement: "%"
    icon: mdi:water-percent-alert
  YOUR_PREFIX_night_humidity_low_alert:
    name: "YOUR_PREFIX Night Humidity Low Alert"
    min: 20
    max: 85
    step: 1
    unit_of_measurement: "%"
    icon: mdi:water-percent-alert

  # CO2 Thresholds (ppm)
  YOUR_PREFIX_day_co2_high_alert:
    name: "YOUR_PREFIX Day CO2 High Alert"
    min: 500
    max: 2500
    step: 50
    unit_of_measurement: "ppm"
    icon: mdi:molecule-co2
  YOUR_PREFIX_day_co2_low_alert:
    name: "YOUR_PREFIX Day CO2 Low Alert"
    min: 300
    max: 1500
    step: 50
    unit_of_measurement: "ppm"
    icon: mdi:molecule-co2
  YOUR_PREFIX_night_co2_high_alert: 
    name: "YOUR_PREFIX Night CO2 High Alert"
    min: 500
    max: 1500 
    step: 50
    unit_of_measurement: "ppm"
    icon: mdi:molecule-co2
  YOUR_PREFIX_night_co2_low_alert: 
    name: "YOUR_PREFIX Night CO2 Low Alert"
    min: 300
    max: 1000
    step: 50
    unit_of_measurement: "ppm"
    icon: mdi:molecule-co2

  # VPD Thresholds (kPa)
  YOUR_PREFIX_day_vpd_high_alert:
    name: "YOUR_PREFIX Day VPD High Alert"
    min: 0.5
    max: 2.0
    step: 0.01
    unit_of_measurement: "kPa"
    icon: mdi:water-thermometer
  YOUR_PREFIX_day_vpd_low_alert:
    name: "YOUR_PREFIX Day VPD Low Alert"
    min: 0.2
    max: 1.5
    step: 0.01
    unit_of_measurement: "kPa"
    icon: mdi:water-thermometer
  YOUR_PREFIX_night_vpd_high_alert:
    name: "YOUR_PREFIX Night VPD High Alert"
    min: 0.4
    max: 1.8
    step: 0.01
    unit_of_measurement: "kPa"
    icon: mdi:water-thermometer
  YOUR_PREFIX_night_vpd_low_alert:
    name: "YOUR_PREFIX Night VPD Low Alert"
    min: 0.1
    max: 1.2
    step: 0.01
    unit_of_measurement: "kPa"
    icon: mdi:water-thermometer

  # Leaf VPD Thresholds (kPa)
  YOUR_PREFIX_day_leaf_vpd_high_alert:
    name: "YOUR_PREFIX Day Leaf VPD High Alert"
    min: 0.5
    max: 2.0
    step: 0.01
    unit_of_measurement: "kPa"
    icon: mdi:leaf
  YOUR_PREFIX_day_leaf_vpd_low_alert:
    name: "YOUR_PREFIX Day Leaf VPD Low Alert"
    min: 0.2
    max: 1.5
    step: 0.01
    unit_of_measurement: "kPa"
    icon: mdi:leaf
  YOUR_PREFIX_night_leaf_vpd_high_alert:
    name: "YOUR_PREFIX Night Leaf VPD High Alert"
    min: 0.4
    max: 1.8
    step: 0.01
    unit_of_measurement: "kPa"
    icon: mdi:leaf
  YOUR_PREFIX_night_leaf_vpd_low_alert:
    name: "YOUR_PREFIX Night Leaf VPD Low Alert"
    min: 0.1
    max: 1.2
    step: 0.01
    unit_of_measurement: "kPa"
    icon: mdi:leaf

  # VWC Thresholds (%) - Volumetric Water Content
  YOUR_PREFIX_day_vwc_low_alert:
    name: "YOUR_PREFIX Day VWC Low Alert"
    min: 10
    max: 70
    step: 1
    unit_of_measurement: "%"
    icon: mdi:water-outline
  YOUR_PREFIX_night_vwc_low_alert:
    name: "YOUR_PREFIX Night VWC Low Alert"
    min: 10
    max: 70
    step: 1
    unit_of_measurement: "%"
    icon: mdi:water-outline

  # pwEC Thresholds (mS/cm) - Pore Water EC
  YOUR_PREFIX_day_pwec_high_alert: 
    name: "YOUR_PREFIX Day pwEC High Alert"
    min: 0.5
    max: 5.0
    step: 0.1
    unit_of_measurement: "mS/cm" 
    icon: mdi:flash-outline
  YOUR_PREFIX_night_pwec_high_alert:
    name: "YOUR_PREFIX Night pwEC High Alert"
    min: 0.5
    max: 5.0
    step: 0.1
    unit_of_measurement: "mS/cm"
    icon: mdi:flash-outline```
4. Create Home Assistant Mute Script
In your Home Assistant scripts.yaml file (or a separate file if you include a scripts directory), add the following script:
```
grow_monitor_mute_sensor:
  alias: "Grow Monitor Mute Sensor Action"
  description: "Fires an event to mute a specific grow monitor sensor."
  fields:
    sensor_key:
      description: "The key of the sensor to mute (e.g., temperature, vwc)."
      example: "temperature"
    duration_hours:
      description: "How many hours to mute the sensor for."
      example: 1
  sequence:
    - event: GROW_MONITOR_MUTE_ACTION 
      event_data:
        sensor_to_mute: "{{ sensor_key }}"
        mute_duration_hours: "{{ duration_hours }}"
  mode: single
```

Reload Home Assistant scripts (Developer Tools > YAML Configuration > Reload Scripts).
5. Configure Entities in grow_monitor.py
Open grow_monitor.py.
At the top, find the ENTITIES dictionary.
Update all entity IDs here to match your specific Home Assistant entities (sensors, and the helpers you just created with YOUR_PREFIX_).
6. Restart AppDaemon
After all files and entities are configured, restart the AppDaemon addon from the Home Assistant UI.
🔧 Using the Monitor
Automatic Operation: The script runs in the background.
Notifications:

When issues are detected and not suppressed by cooldowns or mutes, an AI-summarized notification is sent.
Actions: "Mute [Sensor] (1h)", "Mute [Sensor] (6h)", "Pause ALL Alerts".


Muting: Clicking a mute action silences that specific sensor for the chosen duration. The overall notification system will also enter a 6-hour quiet period (new critical alerts still come through).
Global Pause: Toggle the alerts_paused input_boolean in HA or use the notification action.
📈 Alert Severity & Cooldowns
Severity Levels: IGNORE, NORMAL, URGENT, CRITICAL (determined by violation duration and trend).

A violation must typically last >= 60 seconds to become "NORMAL" severity.


Cooldowns:

Base: 1 hour between notifications.
After Acknowledgment (Mute): 6 hours quiet period.
New Critical Alerts: Bypass cooldowns.


💾 Data Persistence
The script saves its state (muted sensors, last notification times) to grow_monitor_data.pkl in its app_dir (e.g., /config/apps/grow_monitor_data.pkl). This allows state to survive AppDaemon restarts.
🐛 Troubleshooting
No Notifications:

Check AppDaemon logs for errors.
Verify YOUR_PREFIX_environmental_alerts_paused is off.
Confirm time_zone in appdaemon.yaml is correct.
Ensure sensor values violate thresholds for sufficient duration (default >60s for "NORMAL").
Delete grow_monitor_data.pkl and restart AppDaemon to clear stuck mutes.
Test your notify service from HA Developer Tools.


Mute Actions Not Working:

Ensure script.grow_monitor_mute_sensor exists and is correctly configured in HA.
Verify EVENT_GROW_MONITOR_MUTE in grow_monitor.py matches the event: in the HA script.


AI Summary Issues:

Check openai_api_key in apps.yaml.
Look for API errors in AppDaemon logs.


Log Timestamps Incorrect:

Ensure time_zone in the appdaemon.yaml that AppDaemon is actually reading is set to your local timezone.


🌿 Monitor your grow environment effectively with intelligent, summarized, and actionable alerts!Version: 3.1
Last Updated: May 24, 2025
Compatibility: Home Assistant 2023.x+, AppDaemon 4.x+
