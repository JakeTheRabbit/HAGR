# 🤖 Grow Room AI Monitor v2.0 for Home Assistant

An intelligent AppDaemon-based monitoring system for cannabis grow rooms with AI-powered recommendations, smart error detection, and spam-free notifications. Built with pure Python state management - no helper entities required!

## 🚀 Feature Summary

### 🌱 Advanced Environmental Monitoring
- **7 Sensor Types**: Temperature, Humidity, CO2, VPD, Leaf VPD, VWC (moisture), EC (nutrients)
- **Day/Night Awareness**: Different thresholds for lights-on/lights-off periods
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
- **3 Severity Levels**: Normal (5min cooldown), Urgent (2min), Critical (1min)
- **Dynamic Thresholds**: Separate limits for day/night periods
- **Violation Tracking**: Requires sustained violations (1+ minute) before alerting
- **Mobile + TTS Notifications**: Push notifications + voice alerts for critical issues

### 💾 Pure Python Architecture
- **No Helper Entities**: All state managed in Python memory
- **Persistent Storage**: Survives Home Assistant restarts
- **Rolling Data Buffers**: 1 hour sensor history, memory efficient
- **Real-time Processing**: 10-second sensor update intervals

## 📋 Prerequisites

- Home Assistant with AppDaemon add-on
- Environmental sensors configured in HA
- Threshold input helpers (input_number entities)
- Light schedule helpers (input_datetime entities)
- Mobile device with HA app (optional)
- OpenAI API key (optional, for AI features)

## 🛠️ Installation

### 1. Download and Install

**Create the main app file:**
/config/appdaemon/apps/grow_monitor.py
**Configure AppDaemon:**
```yaml
# /config/appdaemon/apps/apps.yaml
grow_monitor:
  module: grow_monitor
  class: GrowRoomAIMonitor
  openai_api_key: !secret openai_api_key  # Optional for AI features
Add OpenAI key (optional):# /config/secrets.yaml
openai_api_key: "sk-your-openai-api-key-here"
2. Create Required Input HelpersLight Schedule (Required)# input_datetime helpers
input_datetime:
  f1_lights_on_time:
    name: "F1 Lights On Time"
    has_time: true
  f1_lights_off_time:
    name: "F1 Lights Off Time" 
    has_time: true

# Alert pause switch
input_boolean:
  f1_environmental_alerts_paused:
    name: "F1 Environmental Alerts Paused"
Temperature Thresholdsinput_number:
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
Additional ThresholdsCreate similar input_number helpers for:
Humidity: day_humidity_high/low_alert, night_humidity_high/low_alert
CO2: day_co2_high/low_alert, night_co2_high/low_alert
VPD: day_vpd_high/low_alert, night_vpd_high/low_alert
Leaf VPD: day_leaf_vpd_high/low_alert, night_leaf_vpd_high/low_alert
VWC: day_vwc_low_alert, night_vwc_low_alert
EC: day_pwec_high_alert, night_pwec_high_alert
3. Configure Your Sensor EntitiesEdit the sensor configuration in grow_monitor.py to match your actual sensor entities:self.sensors = {
    '
