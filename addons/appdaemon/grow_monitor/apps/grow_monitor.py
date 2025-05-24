import appdaemon.plugins.hass.hassapi as hass
import json
import requests
import statistics
import pickle
import os
from datetime import datetime, timedelta, timezone
from dataclasses import dataclass, asdict
from typing import Dict, List, Optional, Any
from collections import deque

# Constants for notification actions
EVENT_GROW_MONITOR_MUTE = "GROW_MONITOR_MUTE_ACTION"
SCRIPT_MUTE_SENSOR = "script.grow_monitor_mute_sensor" # You will need to create this HA script

@dataclass
class SensorReading:
    value: float
    timestamp: datetime

@dataclass
class AlertDetail: # Renamed from AlertContext for clarity
    sensor_name: str
    current_value: float
    unit: str # Added unit for better context
    threshold: float
    trend: str
    severity: str
    alert_start_time: datetime # Changed from violation_duration
    recommendation: str = ""

class GrowRoomAIMonitor(hass.Hass):
    
    ENTITIES = {
        'lights_on': 'input_datetime.f1_lights_on_time',
        'lights_off': 'input_datetime.f1_lights_off_time',
        'alerts_paused': 'input_boolean.f1_environmental_alerts_paused',
        'mobile_notify': 'notify.mobile_app_s23ultra',
        'tts': 'assist_satellite.office_home_assistant_voice_assist_satellite',
        'sensors': {
            'temperature': 'sensor.f1_scd410_back_left_temperature',
            'humidity': 'sensor.f1_scd410_back_left_humidity',
            'co2': 'sensor.f1_scd410_back_left_co2',
            'vpd': 'sensor.f1_scd410_back_left_vpd',
            'leaf_vpd': 'sensor.middle_leaf_vpd',
            'vwc': 'sensor.f1_average_rockwool_vwc',
            'ec': 'sensor.f1_average_rockwool_ec'
        },
        'thresholds': {
            'temp_day_high': 'input_number.f1_day_temp_high_alert', 'temp_day_low': 'input_number.f1_day_temp_low_alert',
            'temp_night_high': 'input_number.f1_night_temp_high_alert', 'temp_night_low': 'input_number.f1_night_temp_low_alert',
            'humidity_day_high': 'input_number.f1_day_humidity_high_alert', 'humidity_day_low': 'input_number.f1_day_humidity_low_alert',
            'humidity_night_high': 'input_number.f1_night_humidity_high_alert', 'humidity_night_low': 'input_number.f1_night_humidity_low_alert',
            'co2_day_high': 'input_number.f1_day_co2_high_alert', 'co2_day_low': 'input_number.f1_day_co2_low_alert',
            'co2_night_high': 'input_number.f1_night_co2_high_alert', 'co2_night_low': 'input_number.f1_night_co2_low_alert',
            'vpd_day_high': 'input_number.f1_day_vpd_high_alert', 'vpd_day_low': 'input_number.f1_day_vpd_low_alert',
            'vpd_night_high': 'input_number.f1_night_vpd_high_alert', 'vpd_night_low': 'input_number.f1_night_vpd_low_alert',
            'leaf_vpd_day_high': 'input_number.f1_day_leaf_vpd_high_alert', 'leaf_vpd_day_low': 'input_number.f1_day_leaf_vpd_low_alert',
            'leaf_vpd_night_high': 'input_number.f1_night_leaf_vpd_high_alert', 'leaf_vpd_night_low': 'input_number.f1_night_leaf_vpd_low_alert',
            'vwc_day_low': 'input_number.f1_day_vwc_low_alert', 'vwc_night_low': 'input_number.f1_night_vwc_low_alert',
            'ec_day_high': 'input_number.f1_day_pwec_high_alert', 'ec_night_high': 'input_number.f1_night_pwec_high_alert'
        }
    }
    
    def initialize(self):
        self.sensors_config = { # Renamed from self.sensors to avoid conflict with AD base class
            'temperature': {'entity': self.ENTITIES['sensors']['temperature'], 'unit': '°C', 'critical_change_rate': 0.1, 'day_high': self.ENTITIES['thresholds']['temp_day_high'], 'day_low': self.ENTITIES['thresholds']['temp_day_low'], 'night_high': self.ENTITIES['thresholds']['temp_night_high'], 'night_low': self.ENTITIES['thresholds']['temp_night_low']},
            'humidity': {'entity': self.ENTITIES['sensors']['humidity'], 'unit': '%', 'critical_change_rate': 2.0, 'day_high': self.ENTITIES['thresholds']['humidity_day_high'], 'day_low': self.ENTITIES['thresholds']['humidity_day_low'], 'night_high': self.ENTITIES['thresholds']['humidity_night_high'], 'night_low': self.ENTITIES['thresholds']['humidity_night_low']},
            'co2': {'entity': self.ENTITIES['sensors']['co2'], 'unit': 'ppm', 'critical_change_rate': 50, 'day_high': self.ENTITIES['thresholds']['co2_day_high'], 'day_low': self.ENTITIES['thresholds']['co2_day_low'], 'night_high': self.ENTITIES['thresholds']['co2_night_high'], 'night_low': self.ENTITIES['thresholds']['co2_night_low']},
            'vpd': {'entity': self.ENTITIES['sensors']['vpd'], 'unit': 'kPa', 'critical_change_rate': 0.1, 'day_high': self.ENTITIES['thresholds']['vpd_day_high'], 'day_low': self.ENTITIES['thresholds']['vpd_day_low'], 'night_high': self.ENTITIES['thresholds']['vpd_night_high'], 'night_low': self.ENTITIES['thresholds']['vpd_night_low']},
            'leaf_vpd': {'entity': self.ENTITIES['sensors']['leaf_vpd'], 'unit': 'kPa', 'critical_change_rate': 0.1, 'day_high': self.ENTITIES['thresholds']['leaf_vpd_day_high'], 'day_low': self.ENTITIES['thresholds']['leaf_vpd_day_low'], 'night_high': self.ENTITIES['thresholds']['leaf_vpd_night_high'], 'night_low': self.ENTITIES['thresholds']['leaf_vpd_night_low']},
            'vwc': {'entity': self.ENTITIES['sensors']['vwc'], 'unit': '%', 'critical_change_rate': 1.0, 'day_low': self.ENTITIES['thresholds']['vwc_day_low'], 'night_low': self.ENTITIES['thresholds']['vwc_night_low']},
            'ec': {'entity': self.ENTITIES['sensors']['ec'], 'unit': 'mS/cm', 'critical_change_rate': 0.2, 'day_high': self.ENTITIES['thresholds']['ec_day_high'], 'night_high': self.ENTITIES['thresholds']['ec_night_high']}
        }
        
        self.sensor_data = {}
        for sensor_name in self.sensors_config: # Use self.sensors_config here
            self.sensor_data[sensor_name] = {
                'history': deque(maxlen=360), 'violation_count': 0, 'last_alert_time': None, 
                'last_notified_value': None, 'previous_value': None, 'trend_history': deque(maxlen=36),
                'is_alerting': False, 'alert_start_time': None, 'user_paused_until': None # Changed from user_paused_until
            }
        
        self.active_alerts_summary: Dict[str, AlertDetail] = {}
        self.user_muted_sensors: Dict[str, datetime] = {} 
        self.last_summary_notification_time: datetime = datetime.min.replace(tzinfo=timezone.utc)
        self.user_acknowledged_at: datetime = datetime.min.replace(tzinfo=timezone.utc)
        self.new_critical_alert_pending = False

        self.alert_history = deque(maxlen=100) 
        self.system_stats = {'total_alerts': 0, 'alerts_by_sensor': {name: 0 for name in self.sensors_config}, 'start_time': datetime.now(timezone.utc)}
        
        self.openai_api_key = self.args.get('openai_api_key', '')
        self.ai_enabled = bool(self.openai_api_key)
        
        # Corrected path for AppDaemon addon config using self.app_dir
        # self.app_dir should point to the directory where this app.py file is located
        self.data_file = os.path.join(self.app_dir, 'grow_monitor_data.pkl')
        
        self.load_persistent_data()
        self.startup_diagnostics()
        
        for sensor_name, config in self.sensors_config.items(): # Use self.sensors_config
            self.listen_state(self.sensor_updated, config['entity'], sensor_name=sensor_name, immediate=True)
        
        self.listen_event(self.handle_mute_action, EVENT_GROW_MONITOR_MUTE)
        
        self.run_every(self.process_and_send_summary_notification, "now+15", 30) 
        self.run_every(self.ai_trend_analysis, "now+300", 300) # Keep AI trend analysis
        self.run_every(self.save_persistent_data, "now+60", 60) 
        self.run_every(self.system_health_check, "now+3600", 3600) 
        
        self.log(f"🌿 Grow Room Monitor v3.1 initialized. Smart notifications active. Monitoring {len(self.sensors_config)} sensors. Data file: {self.data_file}")

    def startup_diagnostics(self):
        self.log("🔍 Running startup diagnostics...")
        pause_state = self.get_state(self.ENTITIES['alerts_paused'])
        self.log(f"🔔 Global alerts paused: {pause_state}")
        
        if self.ai_enabled: self.log("🤖 AI analysis enabled")
        else: self.log("⚠️ AI analysis disabled - no OpenAI API key")
        
        sensors_ok = sum(1 for cfg in self.sensors_config.values() if self.get_state(cfg['entity']) not in [None, 'unavailable', 'unknown'])
        self.log(f"📡 Sensors reporting: {sensors_ok}/{len(self.sensors_config)}")
        
        if self.user_muted_sensors: self.log(f"🔇 Loaded user-muted sensors: {list(self.user_muted_sensors.keys())}")
        else: self.log("✅ No user-muted sensors loaded.")

    def sensor_updated(self, entity, attribute, old, new, kwargs):
        sensor_name = kwargs['sensor_name']
        try:
            current_value = float(new)
        except (ValueError, TypeError):
            self.log(f"⚠️ Invalid sensor value for {sensor_name}: {new}")
            return
        
        now_utc = datetime.now(timezone.utc)
        sensor_data = self.sensor_data[sensor_name]
        sensor_data['history'].append(SensorReading(value=current_value, timestamp=now_utc))
        if len(sensor_data['history']) > 1:
            sensor_data['previous_value'] = sensor_data['history'][-2].value
        
        self.analyze_sensor_state(sensor_name, current_value, now_utc)

    def analyze_sensor_state(self, sensor_name: str, current_value: float, now_utc: datetime):
        sensor_data = self.sensor_data[sensor_name]
        config = self.sensors_config[sensor_name]

        # Check if sensor is user-muted
        if sensor_name in self.user_muted_sensors and now_utc < self.user_muted_sensors[sensor_name]:
            if sensor_data['is_alerting']: 
                self.log(f"🔇 Sensor {sensor_name} is user-muted. Clearing active alert.")
                sensor_data['is_alerting'] = False
                sensor_data['alert_start_time'] = None
                if sensor_name in self.active_alerts_summary:
                    del self.active_alerts_summary[sensor_name]
            return # Do not process further if muted

        is_day = self.is_day_period()
        thresholds = self.get_thresholds(sensor_name, is_day)
        trend = self.calculate_trend(sensor_name)
        violation_type = self.detect_violation(current_value, thresholds)
        
        if violation_type:
            sensor_data['violation_count'] += 1
            severity = self.determine_severity(sensor_name, trend, is_day)

            if severity != "IGNORE":
                if not sensor_data['is_alerting']: 
                    sensor_data['is_alerting'] = True
                    sensor_data['alert_start_time'] = now_utc
                    self.log(f"❗ New alert for {sensor_name}: {severity}, Value: {current_value:.2f}, Trend: {trend}")
                    if severity == "CRITICAL":
                        self.new_critical_alert_pending = True # Flag for immediate notification
                
                # Update or add to active alerts summary
                self.active_alerts_summary[sensor_name] = AlertDetail(
                    sensor_name=sensor_name, current_value=current_value, unit=config['unit'],
                    threshold=thresholds[violation_type], trend=trend, severity=severity,
                    alert_start_time=sensor_data['alert_start_time']
                    # Recommendation will be added by notification function if AI enabled
                )
            else: # Severity is IGNORE
                if sensor_data['is_alerting']: # If it was alerting, clear it
                    self.log(f"✅ Sensor {sensor_name} violation now IGNORE severity. Clearing alert.")
                    sensor_data['is_alerting'] = False
                    sensor_data['alert_start_time'] = None
                    if sensor_name in self.active_alerts_summary:
                        del self.active_alerts_summary[sensor_name]
                sensor_data['violation_count'] = 0 # Reset count if ignored
        else: # No violation
            if sensor_data['is_alerting']: # If it was alerting, clear it
                self.log(f"✅ Sensor {sensor_name} returned to normal. Clearing alert.")
                sensor_data['is_alerting'] = False
                sensor_data['alert_start_time'] = None
                if sensor_name in self.active_alerts_summary:
                    del self.active_alerts_summary[sensor_name]
            sensor_data['violation_count'] = 0 # Reset count
            
    def process_and_send_summary_notification(self, kwargs: Optional[Dict[str, Any]] = None):
        now_utc = datetime.now(timezone.utc)
        
        # Check global AppDaemon pause switch
        if self.get_state(self.ENTITIES['alerts_paused']) == 'on':
            # self.log("⏸️ Global alerts paused via input_boolean. No summary notification.") # Can be noisy
            return

        if not self.active_alerts_summary:
            # self.log("ℹ️ No active alerts. No summary notification needed.") # Too noisy for every 30s
            return

        time_since_last_notification = now_utc - self.last_summary_notification_time
        
        # Smart rate limiting: 6 hours if user acknowledged, 1 hour otherwise
        cooldown_period = timedelta(hours=6) if self.user_acknowledged_at > self.last_summary_notification_time else timedelta(hours=1)
        
        # Send if cooldown passed OR a new critical alert is pending
        if not self.new_critical_alert_pending and time_since_last_notification < cooldown_period:
            # self.log(f"💨 Notification cooldown. Wait: {cooldown_period - time_since_last_notification}") # Can be noisy
            return
            
        # Generate AI summary of the situation (not recommendations)
        ai_summary_text = self.get_ai_situation_summary() if self.ai_enabled else ""
        
        title = "🚨 Grow Room Alert"
        # Use AI summary if available and not empty, otherwise a generic message
        message = ai_summary_text if ai_summary_text else "Environmental issues detected. Check logs for details."
        
        actions = []
        highest_severity_present = "NORMAL"

        # Sort alerts by sensor name for consistent notification order
        sorted_alerts = sorted(self.active_alerts_summary.items(), key=lambda item: item[0]) 

        for sensor_name, alert_detail in sorted_alerts:
            if alert_detail.severity == "CRITICAL": highest_severity_present = "CRITICAL"
            elif alert_detail.severity == "URGENT" and highest_severity_present != "CRITICAL": highest_severity_present = "URGENT"

            # Actions to mute this specific sensor
            actions.append({"action": "call_service", "service": SCRIPT_MUTE_SENSOR, "service_data": {"sensor_key": sensor_name, "duration_hours": 1}, "title": f"Mute {sensor_name} (1h)"})
            actions.append({"action": "call_service", "service": SCRIPT_MUTE_SENSOR, "service_data": {"sensor_key": sensor_name, "duration_hours": 6}, "title": f"Mute {sensor_name} (6h)"})
            # Add more mute options if desired, e.g., 24h
        
        # Action to pause all alerts using the input_boolean
        actions.append({"action": "call_service", "service": "input_boolean.turn_on", "target": {"entity_id": self.ENTITIES['alerts_paused']}, "title": "Pause ALL Alerts"})

        try:
            service_target = self.ENTITIES['mobile_notify'].replace("notify.", "") # Get the service name part
            self.log(f"📨 Sending smart alert: {len(self.active_alerts_summary)} issues. AI Summary: {bool(ai_summary_text)}. Actions: {len(actions)}")
            self.call_service(
                f"notify/{service_target}", # Use correct service call format
                title=title,
                message=message,
                data={
                    "tag": "grow_monitor_summary_alert", # Ensures replacement of previous summary
                    "priority": "high" if highest_severity_present in ["CRITICAL", "URGENT"] else "normal",
                    "actions": actions,
                    "ttl": 0, # Ensure immediate delivery for critical alerts
                    "channel": "Grow Alerts" # Example channel, adjust as needed in HA companion app
                }
            )
            self.last_summary_notification_time = now_utc
            self.new_critical_alert_pending = False # Reset flag after sending
            self.log("✅ Smart notification sent successfully.")
        except Exception as e:
            self.log(f"❌ Error sending smart notification: {e}")

    def get_ai_situation_summary(self) -> str:
        """Generate AI summary of current grow room problems (not recommendations)"""
        if not self.ai_enabled or not self.active_alerts_summary:
            return ""

        # Build context for AI from active_alerts_summary
        issues_context = []
        for alert_detail in self.active_alerts_summary.values(): # Iterate through AlertDetail objects
            duration_str = self.format_duration(datetime.now(timezone.utc) - alert_detail.alert_start_time)
            issues_context.append(
                f"{alert_detail.sensor_name}: {alert_detail.current_value:.1f}{alert_detail.unit} "
                f"(Threshold: {alert_detail.threshold:.1f}, Severity: {alert_detail.severity}, "
                f"Duration: {duration_str}, Trend: {alert_detail.trend})"
            )
        
        prompt = f"""Summarize the following grow room environmental issues in 1-2 concise sentences. Focus on describing WHAT is wrong, not how to fix it. Be professional:

Current issues:
{chr(10).join(issues_context)}

Provide only a brief summary of the problems. No recommendations or conversational fluff."""

        try:
            self.log("🤖 Generating AI situation summary...")
            headers = {
                'Authorization': f'Bearer {self.openai_api_key}',
                'Content-Type': 'application/json',
            }
            data = {
                'model': 'gpt-3.5-turbo', # Cheaper and faster for summaries
                'messages': [{'role': 'user', 'content': prompt}],
                'max_tokens': 100, # Ample for a 1-2 sentence summary
                'temperature': 0.3, # More factual
            }
            response = requests.post(
                'https://api.openai.com/v1/chat/completions',
                headers=headers, json=data, timeout=10 # 10s timeout
            )
            
            if response.status_code == 200:
                ai_response = response.json()
                summary = ai_response['choices'][0]['message']['content'].strip()
                self.log(f"🤖 AI summary generated: {summary[:60]}...") # Log first 60 chars
                return summary
            else:
                self.log(f"❌ AI API error: {response.status_code} - {response.text}")
                return "" # Return empty if AI fails, fallback will be used
                
        except requests.RequestException as e: # Catch network errors
            self.log(f"❌ Network error calling AI API: {e}")
            return ""
        except Exception as e: # Catch other potential errors
            self.log(f"❌ Unexpected error generating AI summary: {e}")
            return ""

    def format_duration(self, duration_td: timedelta) -> str:
        seconds = int(duration_td.total_seconds())
        days, rem_seconds = divmod(seconds, 86400)
        hours, rem_seconds = divmod(rem_seconds, 3600)
        minutes, seconds = divmod(rem_seconds, 60)
        
        parts = []
        if days > 0: parts.append(f"{days}d")
        if hours > 0: parts.append(f"{hours}h")
        if minutes > 0: parts.append(f"{minutes}m")
        # Show seconds only if total duration is less than 5 minutes and no other parts are shown
        if not parts or (days == 0 and hours == 0 and minutes < 5) : 
             parts.append(f"{seconds}s")
        return " ".join(parts) if parts else "0s" # Default to 0s if duration is zero

    def handle_mute_action(self, event_name: str, data: Dict[str, Any], kwargs: Dict[str, Any]):
        self.log(f"🔔 Notification action received: Event: {event_name}, Data: {data}")
        sensor_to_mute = data.get("sensor_to_mute") # Changed from sensor_key for consistency
        duration_hours_str = data.get("mute_duration_hours") # Changed from duration_hours

        if not sensor_to_mute or duration_hours_str is None:
            self.log("⚠️ Invalid mute action data received.")
            return

        try:
            duration_hours = int(duration_hours_str)
        except ValueError:
            self.log(f"⚠️ Invalid duration_hours: {duration_hours_str}")
            return
            
        if sensor_to_mute not in self.sensors_config: # Check against sensors_config
            self.log(f"⚠️ Unknown sensor_key for mute: {sensor_to_mute}")
            return

        now_utc = datetime.now(timezone.utc)
        unpause_time_utc = now_utc + timedelta(hours=duration_hours)
        self.user_muted_sensors[sensor_to_mute] = unpause_time_utc
        
        # Record user acknowledgment for smart rate limiting
        self.user_acknowledged_at = now_utc # This triggers the 6-hour quiet period
        
        # If this sensor was in an active alert, remove it from summary
        if sensor_to_mute in self.active_alerts_summary:
            del self.active_alerts_summary[sensor_to_mute]
            # Also update the sensor_data to reflect it's no longer considered 'alerting' due to mute
            self.sensor_data[sensor_to_mute]['is_alerting'] = False 
            self.sensor_data[sensor_to_mute]['alert_start_time'] = None


        self.log(f"🔇 Sensor '{sensor_to_mute}' muted by user until {unpause_time_utc.isoformat()}. 6hr quiet period active.")
        self.save_persistent_data() # Save the new mute state

        # Trigger an immediate update of the notification (if any alerts remain, or to clear it)
        self.process_and_send_summary_notification()


    def is_day_period(self) -> bool:
        try:
            lights_on_str = self.get_state(self.ENTITIES['lights_on'])
            lights_off_str = self.get_state(self.ENTITIES['lights_off'])
            if not lights_on_str or not lights_off_str: 
                self.log("⚠️ Lights on/off times not set. Defaulting to day period."); return True
            
            on_time = datetime.strptime(lights_on_str, "%H:%M:%S").time()
            off_time = datetime.strptime(lights_off_str, "%H:%M:%S").time()
            
            # Use AppDaemon's self.datetime(aware=True) for timezone-aware current time
            current_time = self.datetime(aware=True).time() 
            
            if on_time < off_time: return on_time <= current_time <= off_time
            else: return current_time >= on_time or current_time <= off_time # Overnight
        except Exception as e:
            self.log(f"❌ Error in is_day_period: {e}. Defaulting to day."); return True

    def get_thresholds(self, sensor_name: str, is_day: bool) -> Dict[str, float]:
        config = self.sensors_config[sensor_name] # Use self.sensors_config
        thresholds = {}
        period = "day" if is_day else "night"
        for th_type in ['high', 'low']:
            key = f"{period}_{th_type}"
            entity_id_for_threshold = config.get(key) # Get entity_id from config
            if entity_id_for_threshold:
                try: 
                    threshold_value = float(self.get_state(entity_id_for_threshold))
                    thresholds[th_type] = threshold_value
                except (ValueError, TypeError): 
                    self.log(f"⚠️ Invalid threshold value for {entity_id_for_threshold} ({sensor_name} {key})")
        return thresholds

    def detect_violation(self, current_value: float, thresholds: Dict[str, float]) -> Optional[str]:
        if 'high' in thresholds and current_value > thresholds['high']: return 'high'
        if 'low' in thresholds and current_value < thresholds['low']: return 'low'
        return None

    def calculate_trend(self, sensor_name: str) -> str:
        history = self.sensor_data[sensor_name]['history']
        if len(history) < 6: return "INSUFFICIENT_DATA" # Need at least 6 readings (1 min)
        
        recent_values = [r.value for r in list(history)[-6:]] # Last 60 seconds
        change = recent_values[-1] - recent_values[0]
        critical_rate = self.sensors_config[sensor_name]['critical_change_rate'] # Use self.sensors_config
        
        if change >= critical_rate * 3: return "RAPID_RISE"
        if change <= -critical_rate * 3: return "RAPID_DROP"
        if change >= critical_rate: return "MODERATE_RISE"
        if change <= -critical_rate: return "MODERATE_DROP"
        return "STABLE"

    def determine_severity(self, sensor_name: str, trend: str, is_day: bool) -> str:
        violation_count = self.sensor_data[sensor_name]['violation_count'] # Consecutive 10s intervals
        
        # Critical conditions
        if sensor_name == 'vwc' and is_day and 'DROP' in trend: return "CRITICAL"
        if sensor_name == 'temperature' and trend == 'RAPID_RISE': return "CRITICAL"
        if violation_count * 10 >= 300 : return "CRITICAL" # Alerting for 5+ minutes
        
        # Urgent conditions
        if 'RAPID' in trend and violation_count * 10 >= 20: return "URGENT" # Rapid change for 20s+
        if violation_count * 10 >= 120: return "URGENT" # Alerting for 2+ minutes
        
        # Normal conditions
        if violation_count * 10 >= 60: return "NORMAL" # Alerting for 1+ minute
        
        return "IGNORE" # Default to ignore if conditions not met for other severities

    def ai_trend_analysis(self, kwargs=None):
        # This is a placeholder for more complex, periodic AI trend analysis.
        # For now, it just logs that it ran. Future enhancements could go here.
        if not self.ai_enabled: return
        self.log("🤖 AI Trend Analysis (periodic check) - currently no specific action beyond per-alert summary.")


    def save_persistent_data(self, kwargs=None):
        try:
            # Prepare sensor_data for saving (convert deques if necessary, though not storing history here)
            s_data_to_save = {
                s_name: {
                    'violation_count': s_d['violation_count'],
                    'last_alert_time': s_d['last_alert_time'].isoformat() if s_d['last_alert_time'] else None,
                    'last_notified_value': s_d['last_notified_value'],
                    'previous_value': s_d['previous_value'],
                    'is_alerting': s_d['is_alerting'], # Save current alerting state
                    'alert_start_time': s_d['alert_start_time'].isoformat() if s_d['alert_start_time'] else None,
                    # 'user_paused_until' is handled by self.user_muted_sensors
                } for s_name, s_d in self.sensor_data.items()
            }
            save_data = {
                'sensor_data_state': s_data_to_save, # Renamed for clarity
                'system_stats': {
                    'total_alerts': self.system_stats['total_alerts'],
                    'alerts_by_sensor': self.system_stats['alerts_by_sensor'],
                    'start_time': self.system_stats['start_time'].isoformat()
                },
                'user_muted_sensors': {k: v.isoformat() for k, v in self.user_muted_sensors.items()},
                'last_summary_notification_time': self.last_summary_notification_time.isoformat(),
                'user_acknowledged_at': self.user_acknowledged_at.isoformat()
            }
            with open(self.data_file, 'wb') as f:
                pickle.dump(save_data, f)
            # self.log("💾 Persistent data saved.") # Can be noisy
        except Exception as e:
            self.log(f"❌ Error saving persistent data to {self.data_file}: {e}")

    def load_persistent_data(self):
        try:
            if not os.path.exists(self.data_file):
                self.log(f"ℹ️ No persistent data file found at {self.data_file}. Starting fresh."); return

            with open(self.data_file, 'rb') as f:
                saved_data = pickle.load(f)
            
            now_utc = datetime.now(timezone.utc)

            loaded_sensor_data_state = saved_data.get('sensor_data_state', {})
            for s_name, s_s_data in loaded_sensor_data_state.items():
                if s_name in self.sensor_data: # Ensure sensor still exists in config
                    self.sensor_data[s_name].update({
                        'violation_count': s_s_data.get('violation_count',0),
                        'last_alert_time': datetime.fromisoformat(s_s_data['last_alert_time']) if s_s_data.get('last_alert_time') else None,
                        'last_notified_value': s_s_data.get('last_notified_value'),
                        'previous_value': s_s_data.get('previous_value'),
                        # 'is_alerting' and 'alert_start_time' will be re-evaluated on first sensor update
                    })
            
            sys_stats = saved_data.get('system_stats', {})
            self.system_stats.update({
                'total_alerts': sys_stats.get('total_alerts',0),
                'alerts_by_sensor': sys_stats.get('alerts_by_sensor', {name: 0 for name in self.sensors_config}),
                'start_time': datetime.fromisoformat(sys_stats['start_time']) if sys_stats.get('start_time') else now_utc
            })

            loaded_muted = saved_data.get('user_muted_sensors', {})
            # Only load mutes that are still in the future
            self.user_muted_sensors = {s_name: datetime.fromisoformat(ts_iso).replace(tzinfo=timezone.utc) 
                                       for s_name, ts_iso in loaded_muted.items() 
                                       if datetime.fromisoformat(ts_iso).replace(tzinfo=timezone.utc) > now_utc}
            
            lsnt_iso = saved_data.get('last_summary_notification_time')
            self.last_summary_notification_time = datetime.fromisoformat(lsnt_iso).replace(tzinfo=timezone.utc) if lsnt_iso else datetime.min.replace(tzinfo=timezone.utc)

            uaa_iso = saved_data.get('user_acknowledged_at')
            self.user_acknowledged_at = datetime.fromisoformat(uaa_iso).replace(tzinfo=timezone.utc) if uaa_iso else datetime.min.replace(tzinfo=timezone.utc)

            self.log(f"📊 Persistent data loaded successfully from {self.data_file}.")
        except Exception as e:
            self.log(f"❌ Error loading persistent data from {self.data_file}: {e}. Starting with fresh data.")
            # Reset to defaults if loading fails to prevent inconsistent state
            self.user_muted_sensors = {}
            self.last_summary_notification_time = datetime.min.replace(tzinfo=timezone.utc)
            self.user_acknowledged_at = datetime.min.replace(tzinfo=timezone.utc)


    def system_health_check(self, kwargs=None):
        uptime = datetime.now(timezone.utc) - self.system_stats['start_time']
        health_report = {
            'uptime': f"{uptime.days}d {int(uptime.total_seconds() // 3600 % 24)}h", # More readable uptime
            'total_alerts_session': self.system_stats['total_alerts'],
            'active_alerts_now': len(self.active_alerts_summary),
            'muted_sensors_now': len(self.user_muted_sensors),
            'sensors_reporting': sum(1 for sd in self.sensor_data.values() if len(sd['history']) > 0)
        }
        self.log(f"🏥 System Health: {health_report}")
        if self.user_muted_sensors: self.log(f"🔇 Currently muted sensors: {list(self.user_muted_sensors.keys())}")
