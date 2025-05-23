```markdown
# 🤖 Grow Room AI Monitor v2.0 for Home Assistant

An intelligent AppDaemon-based monitoring system for cannabis grow rooms with AI-powered recommendations, smart error detection, and spam-free notifications. Built with pure Python state management—no helper entities required!

## 🚀 Feature Summary

### 🌱 Advanced Environmental Monitoring
- **7 Sensor Types**: Temperature, Humidity, CO₂, VPD, Leaf VPD, VWC (moisture), EC (nutrients)  
- **Day/Night Awareness**: Different thresholds for lights-on / lights-off periods  
- **Realistic Range Validation**: Automatically detects sensor malfunctions  
- **Trend Analysis**: RAPID_RISE, MODERATE_DROP, STABLE trend detection  

### 🛡️ Smart Error Detection & Recovery
- **Anti-Spam Protection**: Prevents notifications from malfunctioning sensors  
- **Error State Management**: Sensors enter ERROR state after 3 consecutive bad readings  
- **Automatic Recovery**: Resumes monitoring after 5 consecutive valid readings  
- **Error Notifications**: Separate alerts for sensor failures and recoveries  

### 🧠 AI-Powered Intelligence
- **OpenAI Integration**: Real-time recommendations for each alert  
- **Multi-Sensor Analysis**: Detects patterns across all sensors  
- **Contextual Advice**: Considers current conditions, trends, and grow stage  
- **Periodic Trend Reports**: AI analysis every 5 minutes  

### 🚨 Intelligent Alert System
- **3 Severity Levels**  
  - 🔴 **CRITICAL** (1 min cooldown)  
  - 🟡 **URGENT** (2 min cooldown)  
  - 🟢 **NORMAL** (5 min cooldown)  
- **Dynamic Thresholds**: Separate limits for day/night  
- **Violation Tracking**: Requires sustained violations before alerting  
- **Mobile + TTS Notifications**: Push notifications + voice alerts for critical issues  

### 💾 Pure Python Architecture
- **No Helper Entities**: All state managed in Python memory  
- **Persistent Storage**: Survives Home Assistant restarts  
- **Rolling Data Buffers**: 1 hour sensor history, memory efficient  
- **Real-time Processing**: 10 second sensor update intervals  

## 📋 Prerequisites

- Home Assistant with the AppDaemon add-on  
- Environmental sensors configured in HA  
- Threshold input helpers (`input_number`, `input_datetime`, `input_boolean`)  
- Mobile device with HA app (optional)  
- OpenAI API key (optional, for AI features)  

## 🛠️ Installation

### 1. Download and Install

**Create the main app file:**  
```

/config/appdaemon/apps/grow\_monitor.py

````

**Configure AppDaemon:**  
```yaml
# /config/appdaemon/apps/apps.yaml
grow_monitor:
  module: grow_monitor
  class: GrowRoomAIMonitor
  openai_api_key: !secret openai_api_key  # Optional for AI features
````

**Add OpenAI key (optional):**

```yaml
# /config/secrets.yaml
openai_api_key: "sk-your-openai-api-key-here"
```

### 2. Create Required Input Helpers

**Light Schedule (`input_datetime`):**

```yaml
input_datetime:
  f1_lights_on_time:
    name: "F1 Lights On Time"
    has_time: true
  f1_lights_off_time:
    name: "F1 Lights Off Time"
    has_time: true
```

**Alert Pause Switch (`input_boolean`):**

```yaml
input_boolean:
  f1_environmental_alerts_paused:
    name: "F1 Environmental Alerts Paused"
```

**Temperature Thresholds (`input_number`):**

```yaml
input_number:
  f1_day_temp_high_alert:
    name: "Day Temp High Alert"
    min: 15
    max: 35
    step: 0.5
    unit_of_measurement: "°C"
  f1_day_temp_low_alert:
    name: "Day Temp Low Alert"
    min: 15
    max: 35
    step: 0.5
    unit_of_measurement: "°C"
  f1_night_temp_high_alert:
    name: "Night Temp High Alert"
    min: 15
    max: 35
    step: 0.5
    unit_of_measurement: "°C"
  f1_night_temp_low_alert:
    name: "Night Temp Low Alert"
    min: 15
    max: 35
    step: 0.5
    unit_of_measurement: "°C"
```

**Additional Thresholds:**
Create similar `input_number` helpers for:

* Humidity: `day_humidity_high_alert`, `day_humidity_low_alert`, `night_humidity_high_alert`, `night_humidity_low_alert`
* CO₂: `day_co2_high_alert`, `day_co2_low_alert`, `night_co2_high_alert`, `night_co2_low_alert`
* VPD: `day_vpd_high_alert`, `day_vpd_low_alert`, `night_vpd_high_alert`, `night_vpd_low_alert`
* Leaf VPD: `day_leaf_vpd_high_alert`, `day_leaf_vpd_low_alert`, `night_leaf_vpd_high_alert`, `night_leaf_vpd_low_alert`
* VWC: `day_vwc_low_alert`, `night_vwc_low_alert`
* EC: `day_pwec_high_alert`, `night_pwec_high_alert`

### 3. Configure Your Sensor Entities

Edit the sensor configuration in **grow\_monitor.py** to match your entity IDs:

```python
self.sensors = {
    'temperature': {
        'entity': 'sensor.YOUR_TEMPERATURE_SENSOR',  # ← Update this
        'unit': '°C',
        'critical_change_rate': 0.1,
        'valid_range': (-10.0, 50.0),
        # ... threshold references
    },
    'humidity': {
        'entity': 'sensor.YOUR_HUMIDITY_SENSOR',  # ← Update this
        'unit': '%',
        'critical_change_rate': 2.0,
        'valid_range': (0.0, 100.0),
        # ... threshold references
    },
    # ... configure all 7 sensors
}
```

### 4. Configure Notification Devices

Update these lines with your actual device names:

```python
# Mobile notification device
self.mobile_notify = 'notify.mobile_app_YOUR_DEVICE_NAME'

# TTS device for critical alerts
self.tts_entity = 'YOUR_TTS_ENTITY_ID'
```

### 5. Set Recommended Threshold Values

| Sensor      | Day High  | Day Low | Night High | Night Low |
| ----------- | --------- | ------- | ---------- | --------- |
| Temperature | 28 °C     | 20 °C   | 24 °C      | 18 °C     |
| Humidity    | 65 %      | 45 %    | 70 %       | 50 %      |
| CO₂         | 1800 ppm  | 800 ppm | 1200 ppm   | 400 ppm   |
| VPD         | 1.6 kPa   | 0.8 kPa | 1.2 kPa    | 0.6 kPa   |
| VWC         | —         | 45 %    | —          | 45 %      |
| EC          | 2.5 mS/cm | —       | 2.5 mS/cm  | —         |

## 🔧 Configuration & Error Detection Ranges

Each sensor has built-in realistic ranges for error detection:

| Sensor      | Valid Range         | Purpose                      |
| ----------- | ------------------- | ---------------------------- |
| Temperature | –10 °C to 50 °C     | Detect sensor failures       |
| Humidity    | 0 % to 100 %        | Validate percentage readings |
| CO₂         | 200 ppm to 8000 ppm | Realistic atmospheric range  |
| VPD         | 0.0 kPa to 6.0 kPa  | Physical VPD limits          |
| VWC         | 0 % to 100 %        | Moisture percentage          |
| EC          | 0.0 to 15.0 mS/cm   | Nutrient concentration       |

## 📈 Alert Severity Logic

* 🔴 **CRITICAL** (1 min cooldown)

  * VWC drops during DAY with downward trend
  * Temperature rapid rise (> 0.3 °C/min)
  * Any sensor violating thresholds for ≥ 5 min
* 🟡 **URGENT** (2 min cooldown)

  * Rapid changes (temp > 0.1 °C/min, humidity > 2 %/min, CO₂ > 50 ppm/min)
  * Any violation lasting ≥ 2 min
* 🟢 **NORMAL** (5 min cooldown)

  * Sustained violations lasting ≥ 1 min
  * Moderate changes within thresholds

## 📱 Notification Examples

**Environmental Alert**

```
🚨 URGENT ALERT 🚨
Sensor: TEMPERATURE
Current: 29.2 °C
Threshold: 28.0 °C
Period: DAY
Trend: RAPID_RISE
Duration: 120 s

💡 AI Recommendation:
Increase exhaust fan speed immediately.
Check AC operation and reduce light intensity.
```

**Sensor Error**

```
⚠️ SENSOR ERROR ⚠️
Sensor: HUMIDITY
Issue: Out of range: 150.5
Last Valid: 62.3 %
Alerts PAUSED for this sensor until recovery
Time: 14:32:15
```

**Sensor Recovery**

```
✅ SENSOR RECOVERED ✅
Sensor: HUMIDITY
Current Value: 58.7 %
Error Duration: 0:05:23
Normal monitoring resumed
Time: 14:37:38
```

## 🏥 System Health Monitoring

Hourly Health Reports:

```json
{
  "uptime_hours": 24.5,
  "total_alerts": 28,
  "total_sensor_errors": 3,
  "sensors_in_error": ["humidity"],
  "alerts_by_sensor": {
    "temperature": 8,
    "humidity": 5,
    "co2": 7,
    "vpd": 4,
    "vwc": 4
  },
  "errors_by_sensor": {
    "humidity": 2,
    "temperature": 1
  },
  "sensors_active": 6
}
```

## 🐛 Troubleshooting

### App Not Starting

* Check AppDaemon logs:

  ```bash
  docker logs homeassistant_appdaemon
  ```
* Common issues:

  * File path incorrect: `/config/appdaemon/apps/grow_monitor.py`
  * Sensor entity names don’t match your HA setup
  * Missing threshold helper entities
  * Syntax errors in Python code

### No Notifications Received

* Check alert pause switch: `input_boolean.f1_environmental_alerts_paused = OFF`
* Verify notification device: update `mobile_notify` entity name
* Confirm thresholds: set realistic values
* Wait for sustained violation (≥ 1 min)

### Sensor Stuck in ERROR State

* Verify sensor reporting valid readings in HA
* Check `valid_range` settings
* Restart AppDaemon to reinitialize
* Inspect sensor hardware connections

## 🚀 Performance

* **Memory**: \~50 MB for 7 sensors with full history
* **CPU**: Minimal, event-driven
* **Storage**: < 1 MB persistent data
* **Network**: Only for AI API calls (optional)

**Update Frequency**

* Sensor readings: every 10 s
* Trend analysis: every 5 min
* Health checks: every hour
* Data persistence: every minute

## 📄 License

MIT License – Free for personal and commercial use.

## 🤝 Contributing

Pull requests welcome! Please:

1. Follow existing code patterns
2. Add appropriate logging
3. Test with real sensors
4. Update README for new features
5. Include example configurations

---

🌿 Happy Growing! Monitor your cannabis grow room with confidence using AI-powered intelligence, smart error detection, and spam-free notifications. Your plants will thank you! 🚀

**Version:** 2.0.0
**Last Updated:** 2024
**Compatibility:** Home Assistant 2023.x+, AppDaemon 4.x+

```
```
