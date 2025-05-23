import appdaemon.plugins.hass.hassapi as hass
import json
import requests
import statistics
import pickle
import os
from datetime import datetime, timedelta
from dataclasses import dataclass, asdict
from typing import Dict, List, Optional
from collections import deque

@dataclass
class SensorReading:
    value: float
    timestamp: datetime

@dataclass
class AlertContext:
    sensor_name: str
    current_value: float
    threshold: float
    trend: str
    severity: str
    is_day_period: bool
    violation_duration: int
    recommendation: str = ""

class GrowRoomAIMonitor(hass.Hass):
    
    def initialize(self):
        """Initialize the AI-powered grow room monitor - NO HELPER ENTITIES NEEDED!"""
        
        # Sensor configuration with realistic ranges for error detection
        self.sensors = {
            'temperature': {
                'entity': 'sensor.f1_scd410_back_left_temperature',
                'unit': '°C',
                'critical_change_rate': 0.1,
                'valid_range': (-10.0, 50.0),  # Realistic temp range
                'day_high': 'input_number.f1_day_temp_high_alert',
                'day_low': 'input_number.f1_day_temp_low_alert',
                'night_high': 'input_number.f1_night_temp_high_alert',
                'night_low': 'input_number.f1_night_temp_low_alert'
            },
            'humidity': {
                'entity': 'sensor.f1_scd410_back_left_humidity',
                'unit': '%',
                'critical_change_rate': 2.0,
                'valid_range': (0.0, 100.0),  # Humidity percentage
                'day_high': 'input_number.f1_day_humidity_high_alert',
                'day_low': 'input_number.f1_day_humidity_low_alert',
                'night_high': 'input_number.f1_night_humidity_high_alert',
                'night_low': 'input_number.f1_night_humidity_low_alert'
            },
            'co2': {
                'entity': 'sensor.f1_scd410_back_left_co2',
                'unit': 'ppm',
                'critical_change_rate': 50,
                'valid_range': (200.0, 8000.0),  # CO2 ppm range
                'day_high': 'input_number.f1_day_co2_high_alert',
                'day_low': 'input_number.f1_day_co2_low_alert',
                'night_high': 'input_number.f1_night_co2_high_alert',
                'night_low': 'input_number.f1_night_co2_low_alert'
            },
            'vpd': {
                'entity': 'sensor.f1_scd410_back_left_vpd',
                'unit': 'kPa',
                'critical_change_rate': 0.1,
                'valid_range': (0.0, 6.0),  # VPD kPa range
                'day_high': 'input_number.f1_day_vpd_high_alert',
                'day_low': 'input_number.f1_day_vpd_low_alert',
                'night_high': 'input_number.f1_night_vpd_high_alert',
                'night_low': 'input_number.f1_night_vpd_low_alert'
            },
            'leaf_vpd': {
                'entity': 'sensor.middle_leaf_vpd',
                'unit': 'kPa',
                'critical_change_rate': 0.1,
                'valid_range': (0.0, 6.0),  # Leaf VPD kPa range
                'day_high': 'input_number.f1_day_leaf_vpd_high_alert',
                'day_low': 'input_number.f1_day_leaf_vpd_low_alert',
                'night_high': 'input_number.f1_night_leaf_vpd_high_alert',
                'night_low': 'input_number.f1_night_leaf_vpd_low_alert'
            },
            'vwc': {
                'entity': 'sensor.f1_average_rockwool_vwc',
                'unit': '%',
                'critical_change_rate': 1.0,
                'valid_range': (0.0, 100.0),  # VWC percentage
                'day_low': 'input_number.f1_day_vwc_low_alert',
                'night_low': 'input_number.f1_night_vwc_low_alert'
            },
            'ec': {
                'entity': 'sensor.f1_average_rockwool_ec',
                'unit': 'mS/cm',
                'critical_change_rate': 0.2,
                'valid_range': (0.0, 15.0),  # EC mS/cm range
                'day_high': 'input_number.f1_day_pwec_high_alert',
                'night_high': 'input_number.f1_night_pwec_high_alert'
            }
        }
        
        # ALL DATA STORED IN PYTHON - NO ENTITIES NEEDED!
        self.sensor_data = {}
        for sensor_name in self.sensors:
            self.sensor_data[sensor_name] = {
                'history': deque(maxlen=360),  # 1 hour @ 10s intervals
                'violation_count': 0,
                'last_alert_time': None,
                'last_notified_value': None,
                'previous_value': None,
                'trend_history': deque(maxlen=36),  # 6 minutes of trends
                'error_state': False,  # NEW: Track if sensor is in error
                'error_start_time': None,  # NEW: When error started
                'last_valid_value': None,  # NEW: Last known good value
                'error_count': 0,  # NEW: Count of consecutive errors
                'recovery_notified': False  # NEW: Prevent duplicate recovery notifications
            }
        
        # Global state
        self.alert_history = deque(maxlen=100)
        self.system_stats = {
            'total_alerts': 0,
            'alerts_by_sensor': {name: 0 for name in self.sensors},
            'sensor_errors': {name: 0 for name in self.sensors},  # NEW: Track errors
            'start_time': datetime.now()
        }
        
        # Configuration
        self.lights_on_entity = 'input_datetime.f1_lights_on_time'
        self.lights_off_entity = 'input_datetime.f1_lights_off_time'
        self.alerts_paused_entity = 'input_boolean.f1_environmental_alerts_paused'
        self.openai_api_key = self.args.get('openai_api_key', '')
        self.ai_enabled = bool(self.openai_api_key)
        
        # Notification targets
        self.mobile_notify = 'notify.mobile_app_s23ultra'
        self.tts_entity = 'assist_satellite.office_home_assistant_voice_assist_satellite'
        
        # Data persistence (optional - survives HA restarts)
        self.data_file = '/config/apps/grow_monitor_data.pkl'
        self.load_persistent_data()
        
        # Initialize sensor listeners
        for sensor_name, config in self.sensors.items():
            self.listen_state(self.sensor_updated, config['entity'], 
                            sensor_name=sensor_name, immediate=True)
        
        # Periodic tasks
        self.run_every(self.ai_trend_analysis, "now+300", 300)  # Every 5 minutes
        self.run_every(self.save_persistent_data, "now+60", 60)  # Save data every minute
        self.run_every(self.system_health_check, "now+3600", 3600)  # Hourly health check
        
        self.log("🤖 AI Grow Room Monitor initialized with Error Detection!")
        self.log(f"Monitoring {len(self.sensors)} sensors with AI analysis and error handling")

    def sensor_updated(self, entity, attribute, old, new, kwargs):
        """Handle sensor updates with error detection and pure Python state management"""
        sensor_name = kwargs['sensor_name']
        
        try:
            current_value = float(new)
        except (ValueError, TypeError):
            self.log(f"Error: {sensor_name} sent invalid value: {new}")
            self.handle_sensor_error(sensor_name, f"Invalid value: {new}")
            return
        
        # Check if value is within realistic range
        if not self.is_value_realistic(sensor_name, current_value):
            self.handle_sensor_error(sensor_name, f"Out of range: {current_value}")
            return
        
        # Check for recovery from error state
        if self.sensor_data[sensor_name]['error_state']:
            self.handle_sensor_recovery(sensor_name, current_value)
        
        # Store in Python data structure
        sensor_data = self.sensor_data[sensor_name]
        reading = SensorReading(value=current_value, timestamp=datetime.now())
        sensor_data['history'].append(reading)
        sensor_data['last_valid_value'] = current_value
        sensor_data['error_count'] = 0  # Reset error count on valid reading
        
        # Update previous value for trend calculation
        if len(sensor_data['history']) > 1:
            sensor_data['previous_value'] = sensor_data['history'][-2].value
        
        # Only analyze if sensor is not in error state
        if not sensor_data['error_state']:
            self.analyze_sensor(sensor_name, current_value)

    def is_value_realistic(self, sensor_name: str, value: float) -> bool:
        """Check if sensor value is within realistic range"""
        valid_range = self.sensors[sensor_name]['valid_range']
        return valid_range[0] <= value <= valid_range[1]

    def handle_sensor_error(self, sensor_name: str, error_detail: str):
        """Handle sensor error state"""
        sensor_data = self.sensor_data[sensor_name]
        sensor_data['error_count'] += 1
        
        # Require 3 consecutive errors before declaring ERROR state
        if sensor_data['error_count'] >= 3 and not sensor_data['error_state']:
            sensor_data['error_state'] = True
            sensor_data['error_start_time'] = datetime.now()
            sensor_data['violation_count'] = 0  # Reset violation count
            sensor_data['recovery_notified'] = False
            
            # Track in system stats
            self.system_stats['sensor_errors'][sensor_name] += 1
            
            # Send error notification
            self.send_error_notification(sensor_name, error_detail)
            
            self.log(f"⚠️ SENSOR ERROR: {sensor_name} - {error_detail}")
        
        elif sensor_data['error_count'] < 3:
            self.log(f"Warning: {sensor_name} error #{sensor_data['error_count']} - {error_detail}")

    def handle_sensor_recovery(self, sensor_name: str, current_value: float):
        """Handle sensor recovery from error state"""
        sensor_data = self.sensor_data[sensor_name]
        
        # Reset error count on valid reading
        sensor_data['error_count'] = 0
        
        # Check if we should exit error state (require 5 consecutive good readings)
        recent_valid_count = 0
        if len(sensor_data['history']) >= 5:
            recent_values = [r.value for r in list(sensor_data['history'])[-5:]]
            recent_valid_count = sum(1 for v in recent_values 
                                   if self.is_value_realistic(sensor_name, v))
        
        # Exit error state if we have enough valid readings
        if recent_valid_count >= 5 and not sensor_data['recovery_notified']:
            sensor_data['error_state'] = False
            error_duration = datetime.now() - sensor_data['error_start_time']
            sensor_data['error_start_time'] = None
            sensor_data['recovery_notified'] = True
            
            # Send recovery notification
            self.send_recovery_notification(sensor_name, current_value, error_duration)
            
            self.log(f"✅ SENSOR RECOVERED: {sensor_name} - back to normal operation")

    def send_error_notification(self, sensor_name: str, error_detail: str):
        """Send notification when sensor enters error state"""
        unit = self.sensors[sensor_name]['unit']
        last_valid = self.sensor_data[sensor_name]['last_valid_value']
        
        message = f"⚠️ SENSOR ERROR ⚠️\n"
        message += f"Sensor: {sensor_name.upper()}\n"
        message += f"Issue: {error_detail}\n"
        message += f"Last Valid: {last_valid:.2f}{unit} " if last_valid else "No previous valid reading\n"
        message += f"Alerts PAUSED for this sensor until recovery\n"
        message += f"Time: {datetime.now().strftime('%H:%M:%S')}"
        
        try:
            self.call_service("notify/" + self.mobile_notify.split(".")[1], 
                            title=f"SENSOR ERROR: {sensor_name}",
                            message=message,
                            data={
                                "priority": "high",
                                "tag": f"sensor_error_{sensor_name}",
                                "actions": [
                                    {"action": "check_sensor", "title": "Check Sensor"},
                                    {"action": "pause_all_alerts", "title": "Pause All Alerts"}
                                ]
                            })
        except Exception as e:
            self.log(f"Error sending sensor error notification: {e}")

    def send_recovery_notification(self, sensor_name: str, current_value: float, error_duration: timedelta):
        """Send notification when sensor recovers from error state"""
        unit = self.sensors[sensor_name]['unit']
        duration_str = str(error_duration).split('.')[0]  # Remove microseconds
        
        message = f"✅ SENSOR RECOVERED ✅\n"
        message += f"Sensor: {sensor_name.upper()}\n"
        message += f"Current Value: {current_value:.2f}{unit}\n"
        message += f"Error Duration: {duration_str}\n"
        message += f"Normal monitoring resumed\n"
        message += f"Time: {datetime.now().strftime('%H:%M:%S')}"
        
        try:
            self.call_service("notify/" + self.mobile_notify.split(".")[1], 
                            title=f"RECOVERY: {sensor_name}",
                            message=message,
                            data={
                                "priority": "normal",
                                "tag": f"sensor_recovery_{sensor_name}",
                                "actions": [
                                    {"action": "view_trends", "title": "View Trends"}
                                ]
                            })
        except Exception as e:
            self.log(f"Error sending sensor recovery notification: {e}")

    def analyze_sensor(self, sensor_name: str, current_value: float):
        """Pure Python analysis - no entity dependencies - SKIP SENSORS IN ERROR STATE"""
        
        # Skip analysis if sensor is in error state
        if self.sensor_data[sensor_name]['error_state']:
            return
        
        if self.get_state(self.alerts_paused_entity) == 'on':
            return
        
        sensor_data = self.sensor_data[sensor_name]
        
        # Skip if insufficient data
        if len(sensor_data['history']) < 2:
            return
        
        # Get current period and thresholds
        is_day = self.is_day_period()
        thresholds = self.get_thresholds(sensor_name, is_day)
        
        # Trend analysis
        trend = self.calculate_trend(sensor_name)
        
        # Violation detection
        violation_type = self.detect_violation(current_value, thresholds)
        
        if violation_type:
            sensor_data['violation_count'] += 1
        else:
            sensor_data['violation_count'] = 0
            return
        
        # Severity determination
        severity = self.determine_severity(sensor_name, trend, is_day)
        
        if severity == "IGNORE":
            return
        
        # Cooldown check
        if not self.cooldown_expired(sensor_name, severity):
            return
        
        # Create and send alert
        alert_context = AlertContext(
            sensor_name=sensor_name,
            current_value=current_value,
            threshold=thresholds[violation_type],
            trend=trend,
            severity=severity,
            is_day_period=is_day,
            violation_duration=sensor_data['violation_count'] * 10
        )
        
        # AI recommendation
        if self.ai_enabled:
            alert_context.recommendation = self.get_ai_recommendation(alert_context)
        
        # Send alerts
        self.send_alert(alert_context)
        
        # Update state in Python
        sensor_data['last_alert_time'] = datetime.now()
        sensor_data['last_notified_value'] = current_value
        self.alert_history.append(alert_context)
        self.system_stats['total_alerts'] += 1
        self.system_stats['alerts_by_sensor'][sensor_name] += 1

    def is_day_period(self) -> bool:
        """Determine if it's currently day period based on lights schedule"""
        try:
            lights_on = self.get_state(self.lights_on_entity)
            lights_off = self.get_state(self.lights_off_entity)
            
            # Parse times
            on_time = datetime.strptime(lights_on, "%H:%M:%S").time()
            off_time = datetime.strptime(lights_off, "%H:%M:%S").time()
            current_time = datetime.now().time()
            
            # Handle day/night cycle
            if on_time < off_time:
                # Normal day (e.g., 06:00 to 22:00)
                return on_time <= current_time <= off_time
            else:
                # Overnight lights (e.g., 22:00 to 06:00)
                return current_time >= on_time or current_time <= off_time
                
        except Exception as e:
            self.log(f"Error determining day period: {e}")
            # Default to day period if error
            return True

    def get_thresholds(self, sensor_name: str, is_day: bool) -> Dict[str, float]:
        """Get threshold values for the sensor and time period"""
        config = self.sensors[sensor_name]
        thresholds = {}
        
        # Build threshold dictionary based on available thresholds
        period = "day" if is_day else "night"
        
        for threshold_type in ['high', 'low']:
            threshold_key = f"{period}_{threshold_type}"
            if threshold_key in config:
                try:
                    value = float(self.get_state(config[threshold_key]))
                    thresholds[threshold_type] = value
                except (ValueError, TypeError):
                    self.log(f"Error getting threshold {threshold_key} for {sensor_name}")
        
        return thresholds

    def detect_violation(self, current_value: float, thresholds: Dict[str, float]) -> Optional[str]:
        """Detect if current value violates any threshold"""
        if 'high' in thresholds and current_value > thresholds['high']:
            return 'high'
        elif 'low' in thresholds and current_value < thresholds['low']:
            return 'low'
        return None

    def calculate_trend(self, sensor_name: str) -> str:
        """Calculate trend using pure Python"""
        history = self.sensor_data[sensor_name]['history']
        
        if len(history) < 6:
            return "INSUFFICIENT_DATA"
        
        # Get last 6 readings (1 minute)
        recent_values = [r.value for r in list(history)[-6:]]
        
        # Calculate change rate
        change = recent_values[-1] - recent_values[0]
        critical_rate = self.sensors[sensor_name]['critical_change_rate']
        
        # Classify trend
        if change >= critical_rate * 3:
            return "RAPID_RISE"
        elif change <= -critical_rate * 3:
            return "RAPID_DROP"
        elif change >= critical_rate:
            return "MODERATE_RISE"
        elif change <= -critical_rate:
            return "MODERATE_DROP"
        else:
            return "STABLE"

    def determine_severity(self, sensor_name: str, trend: str, is_day: bool) -> str:
        """Determine alert severity"""
        violation_count = self.sensor_data[sensor_name]['violation_count']
        
        # Critical scenarios
        if sensor_name == 'vwc' and is_day and 'DROP' in trend:
            return "CRITICAL"
        
        if sensor_name == 'temperature' and trend == 'RAPID_RISE':
            return "CRITICAL"
        
        if violation_count >= 30:  # 5+ minutes
            return "CRITICAL"
        
        # Urgent scenarios
        if 'RAPID' in trend and violation_count >= 2:
            return "URGENT"
        
        if violation_count >= 12:  # 2+ minutes
            return "URGENT"
        
        # Normal alerts
        if violation_count >= 6:  # 1+ minute
            return "NORMAL"
        
        return "IGNORE"

    def cooldown_expired(self, sensor_name: str, severity: str) -> bool:
        """Check cooldown using Python datetime"""
        last_alert = self.sensor_data[sensor_name]['last_alert_time']
        if not last_alert:
            return True
        
        cooldown_seconds = {
            'CRITICAL': 60,   # 1 minute
            'URGENT': 120,    # 2 minutes  
            'NORMAL': 300     # 5 minutes
        }.get(severity, 300)
        
        return (datetime.now() - last_alert).total_seconds() >= cooldown_seconds

    def send_alert(self, context: AlertContext):
        """Send notifications based on severity"""
        # Format message
        unit = self.sensors[context.sensor_name]['unit']
        period = "DAY" if context.is_day_period else "NIGHT"
        
        message = f"🚨 {context.severity} ALERT 🚨\n"
        message += f"Sensor: {context.sensor_name.upper()}\n"
        message += f"Current: {context.current_value:.2f}{unit}\n"
        message += f"Threshold: {context.threshold:.2f}{unit}\n"
        message += f"Period: {period}\n"
        message += f"Trend: {context.trend}\n"
        message += f"Duration: {context.violation_duration}s\n"
        
        if context.recommendation:
            message += f"\n💡 AI Recommendation:\n{context.recommendation}"
        
        # Mobile notification (always send)
        try:
            self.call_service("notify/" + self.mobile_notify.split(".")[1], 
                            title=f"{context.severity}: {context.sensor_name}",
                            message=message,
                            data={
                                "priority": "high" if context.severity in ["CRITICAL", "URGENT"] else "normal",
                                "tag": f"grow_alert_{context.sensor_name}",
                                "actions": [
                                    {"action": "pause_alerts", "title": "Pause Alerts"},
                                    {"action": "view_trends", "title": "View Trends"}
                                ]
                            })
        except Exception as e:
            self.log(f"Error sending mobile notification: {e}")
        
        # TTS for critical alerts
        if context.severity == "CRITICAL":
            try:
                tts_message = f"Critical grow room alert. {context.sensor_name} is {context.current_value:.1f} {unit}."
                self.call_service("tts/cloud_say",
                                entity_id=self.tts_entity,
                                message=tts_message)
            except Exception as e:
                self.log(f"Error sending TTS: {e}")
        
        self.log(f"Alert sent: {context.sensor_name} {context.severity} - {context.current_value}{unit}")

    def get_ai_recommendation(self, context: AlertContext) -> str:
        """AI recommendations using current sensor data"""
        if not self.ai_enabled:
            return "AI unavailable"
        
        try:
            # Build context from Python data structures
            sensor_context = {}
            for name, data in self.sensor_data.items():
                if len(data['history']) > 0 and not data['error_state']:
                    recent_values = [r.value for r in list(data['history'])[-10:]]
                    sensor_context[name] = {
                        'current': recent_values[-1] if recent_values else None,
                        'trend': self.calculate_trend(name),
                        'recent_readings': recent_values,
                        'error_state': data['error_state']
                    }
            
            # Prepare AI prompt
            prompt = f"""
            You are an expert cannabis grow room consultant. Analyze this alert and provide specific actionable recommendations.
            
            ALERT DETAILS:
            - Sensor: {context.sensor_name}
            - Current Value: {context.current_value}{self.sensors[context.sensor_name]['unit']}
            - Threshold Violated: {context.threshold}{self.sensors[context.sensor_name]['unit']}
            - Trend: {context.trend}
            - Severity: {context.severity}
            - Time Period: {'Day' if context.is_day_period else 'Night'}
            - Violation Duration: {context.violation_duration} seconds
            
            ALL SENSOR READINGS:
            {json.dumps(sensor_context, indent=2)}
            
            Provide a concise recommendation (max 200 characters) focusing on immediate actionable steps.
            """
            
            # Call OpenAI API
            headers = {
                'Authorization': f'Bearer {self.openai_api_key}',
                'Content-Type': 'application/json'
            }
            
            data = {
                'model': 'gpt-4o-mini',
                'messages': [
                    {'role': 'system', 'content': 'You are a concise cannabis grow expert. Provide brief, actionable recommendations.'},
                    {'role': 'user', 'content': prompt}
                ],
                'max_tokens': 100,
                'temperature': 0.7
            }
            
            response = requests.post('https://api.openai.com/v1/chat/completions',
                                   headers=headers, json=data, timeout=10)
            
            if response.status_code == 200:
                result = response.json()
                return result['choices'][0]['message']['content'].strip()
            else:
                self.log(f"AI API error: {response.status_code}")
                return "AI temporarily unavailable"
                
        except Exception as e:
            self.log(f"Error getting AI recommendation: {e}")
            return "AI error occurred"

    def ai_trend_analysis(self, kwargs=None):
        """Periodic AI analysis of trends and patterns"""
        if not self.ai_enabled:
            return
        
        try:
            # Analyze patterns across all sensors (exclude error sensors)
            patterns = {}
            for sensor_name, data in self.sensor_data.items():
                if len(data['history']) >= 30 and not data['error_state']:
                    values = [r.value for r in list(data['history'])[-30:]]
                    patterns[sensor_name] = {
                        'mean': statistics.mean(values),
                        'stdev': statistics.stdev(values) if len(values) > 1 else 0,
                        'trend': self.calculate_trend(sensor_name),
                        'recent_violations': data['violation_count']
                    }
            
            # Only analyze if we have significant patterns
            if len(patterns) < 3:
                return
            
            # Check for concerning multi-sensor patterns
            high_variability_sensors = [name for name, data in patterns.items() 
                                      if data['stdev'] > data['mean'] * 0.1]
            
            trending_sensors = [name for name, data in patterns.items() 
                              if 'RAPID' in data['trend']]
            
            # Send summary if concerning patterns detected
            if len(high_variability_sensors) >= 2 or len(trending_sensors) >= 2:
                summary = f"🔍 AI Trend Analysis:\n"
                summary += f"High variability: {', '.join(high_variability_sensors)}\n"
                summary += f"Trending sensors: {', '.join(trending_sensors)}\n"
                summary += f"Total alerts: {self.system_stats['total_alerts']}\n"
                
                self.log(f"AI Trend Summary: {summary}")
                
        except Exception as e:
            self.log(f"Error in AI trend analysis: {e}")

    def save_persistent_data(self, kwargs=None):
        """Save data to survive HA restarts"""
        try:
            # Convert deques to lists for pickling
            save_data = {
                'sensor_data': {},
                'system_stats': self.system_stats
            }
            
            for sensor_name, data in self.sensor_data.items():
                save_data['sensor_data'][sensor_name] = {
                    'violation_count': data['violation_count'],
                    'last_alert_time': data['last_alert_time'],
                    'last_notified_value': data['last_notified_value'],
                    'previous_value': data['previous_value'],
                    'error_state': data['error_state'],
                    'error_start_time': data['error_start_time'],
                    'last_valid_value': data['last_valid_value'],
                    'error_count': data['error_count']
                    # Don't save full history - too large
                }
            
            with open(self.data_file, 'wb') as f:
                pickle.dump(save_data, f)
                
        except Exception as e:
            self.log(f"Error saving data: {e}")

    def load_persistent_data(self):
        """Load data from previous sessions"""
        try:
            if os.path.exists(self.data_file):
                with open(self.data_file, 'rb') as f:
                    saved_data = pickle.load(f)
                
                # Restore sensor data
                for sensor_name, data in saved_data.get('sensor_data', {}).items():
                    if sensor_name in self.sensor_data:
                        self.sensor_data[sensor_name].update(data)
                
                # Restore system stats
                if 'system_stats' in saved_data:
                    self.system_stats.update(saved_data['system_stats'])
                
                self.log("📊 Persistent data loaded successfully")
                
        except Exception as e:
            self.log(f"Error loading data: {e}")

    def system_health_check(self, kwargs=None):
        """Periodic system health reporting"""
        uptime = datetime.now() - self.system_stats['start_time']
        
        # Count sensors in error state
        error_sensors = [name for name, data in self.sensor_data.items() if data['error_state']]
        
        health_report = {
            'uptime_hours': uptime.total_seconds() / 3600,
            'total_alerts': self.system_stats['total_alerts'],
            'total_sensor_errors': sum(self.system_stats['sensor_errors'].values()),
            'sensors_in_error': error_sensors,
            'alerts_by_sensor': self.system_stats['alerts_by_sensor'],
            'errors_by_sensor': self.system_stats['sensor_errors'],
            'sensors_active': len([s for s in self.sensor_data.values() 
                                 if len(s['history']) > 0 and not s['error_state']])
        }
        
        self.log(f"🏥 System Health: {health_report}")
        
        # Send notification if multiple sensors in error
        if len(error_sensors) >= 2:
            error_message = f"⚠️ MULTIPLE SENSOR ERRORS ⚠️\n"
            error_message += f"Sensors in error: {', '.join(error_sensors)}\n"
            error_message += f"Check sensor connections and power\n"
            error_message += f"System uptime: {uptime.total_seconds() / 3600:.1f}h"
            
            try:
                self.call_service("notify/" + self.mobile_notify.split(".")[1], 
                                title="SYSTEM ALERT: Multiple Sensor Errors",
                                message=error_message,
                                data={"priority": "high"})
            except Exception as e:
                self.log(f"Error sending system health notification: {e}")
