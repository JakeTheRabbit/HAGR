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
    
    # HOME ASSISTANT ENTITY CONFIGURATION - EASY TO MODIFY
    ENTITIES = {
        # Control entities
        'lights_on': 'input_datetime.f1_lights_on_time',
        'lights_off': 'input_datetime.f1_lights_off_time',
        'alerts_paused': 'input_boolean.f1_environmental_alerts_paused',
        
        # Notification targets
        'mobile_notify': 'notify.mobile_app_s23ultra',
        'tts': 'assist_satellite.office_home_assistant_voice_assist_satellite',
        
        # Environmental sensors
        'sensors': {
            'temperature': 'sensor.f1_scd410_back_left_temperature',
            'humidity': 'sensor.f1_scd410_back_left_humidity',
            'co2': 'sensor.f1_scd410_back_left_co2',
            'vpd': 'sensor.f1_scd410_back_left_vpd',
            'leaf_vpd': 'sensor.middle_leaf_vpd',
            'vwc': 'sensor.f1_average_rockwool_vwc',
            'ec': 'sensor.f1_average_rockwool_ec'
        },
        
        # Alert threshold entities
        'thresholds': {
            'temp_day_high': 'input_number.f1_day_temp_high_alert',
            'temp_day_low': 'input_number.f1_day_temp_low_alert',
            'temp_night_high': 'input_number.f1_night_temp_high_alert',
            'temp_night_low': 'input_number.f1_night_temp_low_alert',
            'humidity_day_high': 'input_number.f1_day_humidity_high_alert',
            'humidity_day_low': 'input_number.f1_day_humidity_low_alert',
            'humidity_night_high': 'input_number.f1_night_humidity_high_alert',
            'humidity_night_low': 'input_number.f1_night_humidity_low_alert',
            'co2_day_high': 'input_number.f1_day_co2_high_alert',
            'co2_day_low': 'input_number.f1_day_co2_low_alert',
            'co2_night_high': 'input_number.f1_night_co2_high_alert',
            'co2_night_low': 'input_number.f1_night_co2_low_alert',
            'vpd_day_high': 'input_number.f1_day_vpd_high_alert',
            'vpd_day_low': 'input_number.f1_day_vpd_low_alert',
            'vpd_night_high': 'input_number.f1_night_vpd_high_alert',
            'vpd_night_low': 'input_number.f1_night_vpd_low_alert',
            'leaf_vpd_day_high': 'input_number.f1_day_leaf_vpd_high_alert',
            'leaf_vpd_day_low': 'input_number.f1_day_leaf_vpd_low_alert',
            'leaf_vpd_night_high': 'input_number.f1_night_leaf_vpd_high_alert',
            'leaf_vpd_night_low': 'input_number.f1_night_leaf_vpd_low_alert',
            'vwc_day_low': 'input_number.f1_day_vwc_low_alert',
            'vwc_night_low': 'input_number.f1_night_vwc_low_alert',
            'ec_day_high': 'input_number.f1_day_pwec_high_alert',
            'ec_night_high': 'input_number.f1_night_pwec_high_alert'
        }
    }
    
    def initialize(self):
        """Initialize the AI-powered grow room monitor"""
        
        # Sensor configuration using centralized entities
        self.sensors = {
            'temperature': {
                'entity': self.ENTITIES['sensors']['temperature'],
                'unit': '°C',
                'critical_change_rate': 0.1,
                'day_high': self.ENTITIES['thresholds']['temp_day_high'],
                'day_low': self.ENTITIES['thresholds']['temp_day_low'],
                'night_high': self.ENTITIES['thresholds']['temp_night_high'],
                'night_low': self.ENTITIES['thresholds']['temp_night_low']
            },
            'humidity': {
                'entity': self.ENTITIES['sensors']['humidity'],
                'unit': '%',
                'critical_change_rate': 2.0,
                'day_high': self.ENTITIES['thresholds']['humidity_day_high'],
                'day_low': self.ENTITIES['thresholds']['humidity_day_low'],
                'night_high': self.ENTITIES['thresholds']['humidity_night_high'],
                'night_low': self.ENTITIES['thresholds']['humidity_night_low']
            },
            'co2': {
                'entity': self.ENTITIES['sensors']['co2'],
                'unit': 'ppm',
                'critical_change_rate': 50,
                'day_high': self.ENTITIES['thresholds']['co2_day_high'],
                'day_low': self.ENTITIES['thresholds']['co2_day_low'],
                'night_high': self.ENTITIES['thresholds']['co2_night_high'],
                'night_low': self.ENTITIES['thresholds']['co2_night_low']
            },
            'vpd': {
                'entity': self.ENTITIES['sensors']['vpd'],
                'unit': 'kPa',
                'critical_change_rate': 0.1,
                'day_high': self.ENTITIES['thresholds']['vpd_day_high'],
                'day_low': self.ENTITIES['thresholds']['vpd_day_low'],
                'night_high': self.ENTITIES['thresholds']['vpd_night_high'],
                'night_low': self.ENTITIES['thresholds']['vpd_night_low']
            },
            'leaf_vpd': {
                'entity': self.ENTITIES['sensors']['leaf_vpd'],
                'unit': 'kPa',
                'critical_change_rate': 0.1,
                'day_high': self.ENTITIES['thresholds']['leaf_vpd_day_high'],
                'day_low': self.ENTITIES['thresholds']['leaf_vpd_day_low'],
                'night_high': self.ENTITIES['thresholds']['leaf_vpd_night_high'],
                'night_low': self.ENTITIES['thresholds']['leaf_vpd_night_low']
            },
            'vwc': {
                'entity': self.ENTITIES['sensors']['vwc'],
                'unit': '%',
                'critical_change_rate': 1.0,
                'day_low': self.ENTITIES['thresholds']['vwc_day_low'],
                'night_low': self.ENTITIES['thresholds']['vwc_night_low']
            },
            'ec': {
                'entity': self.ENTITIES['sensors']['ec'],
                'unit': 'mS/cm',
                'critical_change_rate': 0.2,
                'day_high': self.ENTITIES['thresholds']['ec_day_high'],
                'night_high': self.ENTITIES['thresholds']['ec_night_high']
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
                'trend_history': deque(maxlen=36)  # 6 minutes of trends
            }
        
        # Global state
        self.alert_history = deque(maxlen=100)
        self.system_stats = {
            'total_alerts': 0,
            'alerts_by_sensor': {name: 0 for name in self.sensors},
            'start_time': datetime.now()
        }
        
        # Configuration using centralized entities
        self.lights_on_entity = self.ENTITIES['lights_on']
        self.lights_off_entity = self.ENTITIES['lights_off']
        self.alerts_paused_entity = self.ENTITIES['alerts_paused']
        self.mobile_notify = self.ENTITIES['mobile_notify']
        self.tts_entity = self.ENTITIES['tts']
        
        # Enhanced AI configuration with debugging
        self.openai_api_key = self.args.get('openai_api_key', '')
        self.ai_enabled = bool(self.openai_api_key)
        
        # Data persistence (optional - survives HA restarts)
        self.data_file = '/config/appdaemon/apps/grow_monitor_data.pkl'
        self.load_persistent_data()
        
        # Enhanced startup diagnostics
        self.startup_diagnostics()
        
        # Initialize sensor listeners
        for sensor_name, config in self.sensors.items():
            self.listen_state(self.sensor_updated, config['entity'], 
                            sensor_name=sensor_name, immediate=True)
        
        # Periodic tasks
        self.run_every(self.ai_trend_analysis, "now+300", 300)  # Every 5 minutes
        self.run_every(self.save_persistent_data, "now+60", 60)  # Save data every minute
        self.run_every(self.system_health_check, "now+3600", 3600)  # Hourly health check
        
        self.log("🤖 AI Grow Room Monitor initialized successfully!")
        self.log(f"Monitoring {len(self.sensors)} sensors with AI analysis")

    def startup_diagnostics(self):
        """Enhanced startup diagnostics to debug issues"""
        self.log("🔍 Running startup diagnostics...")
        
        # Check alert pause state
        try:
            pause_state = self.get_state(self.alerts_paused_entity)
            self.log(f"📱 Alert pause state: {pause_state} (entity: {self.alerts_paused_entity})")
        except Exception as e:
            self.log(f"❌ Error reading alert pause state: {e}")
        
        # Check AI configuration
        if self.openai_api_key:
            key_preview = f"{self.openai_api_key[:8]}..." if len(self.openai_api_key) > 8 else "TOO SHORT"
            self.log(f"🤖 AI enabled: {self.ai_enabled} (key: {key_preview})")
        else:
            self.log("❌ OpenAI API key not configured - AI disabled")
        
        # Check sensor entities
        missing_sensors = []
        for sensor_name, config in self.sensors.items():
            try:
                state = self.get_state(config['entity'])
                if state is None:
                    missing_sensors.append(f"{sensor_name}:{config['entity']}")
            except Exception as e:
                missing_sensors.append(f"{sensor_name}:{config['entity']} (error: {e})")
        
        if missing_sensors:
            self.log(f"⚠️ Missing/broken sensors: {missing_sensors}")
        else:
            self.log("✅ All sensors accessible")
        
        # Check lights schedule
        try:
            lights_on = self.get_state(self.lights_on_entity)
            lights_off = self.get_state(self.lights_off_entity)
            is_day = self.is_day_period()
            self.log(f"☀️ Lights schedule: ON={lights_on}, OFF={lights_off}, Current period={'DAY' if is_day else 'NIGHT'}")
        except Exception as e:
            self.log(f"❌ Error checking lights schedule: {e}")

    def sensor_updated(self, entity, attribute, old, new, kwargs):
        """Handle sensor updates with enhanced debugging"""
        sensor_name = kwargs['sensor_name']
        
        try:
            current_value = float(new)
        except (ValueError, TypeError):
            self.log(f"⚠️ Invalid sensor value for {sensor_name}: {new}")
            return
        
        # Store in Python data structure
        sensor_data = self.sensor_data[sensor_name]
        reading = SensorReading(value=current_value, timestamp=datetime.now())
        sensor_data['history'].append(reading)
        
        # Update previous value for trend calculation
        if len(sensor_data['history']) > 1:
            sensor_data['previous_value'] = sensor_data['history'][-2].value
        
        # Analyze this reading
        self.analyze_sensor(sensor_name, current_value)

    def analyze_sensor(self, sensor_name: str, current_value: float):
        """Enhanced analysis with better debugging for alert pausing"""
        
        # Enhanced alert pause check with debugging
        try:
            pause_state = self.get_state(self.alerts_paused_entity)
            self.log(f"🔍 Alert pause check - Entity: {self.alerts_paused_entity}, State: '{pause_state}', Type: {type(pause_state)}")
            
            if pause_state == 'on':
                self.log(f"⏸️ Alerts paused - skipping analysis for {sensor_name}")
                return
            elif pause_state is None:
                self.log(f"❌ Alert pause entity not found: {self.alerts_paused_entity}")
            elif pause_state != 'off':
                self.log(f"⚠️ Unexpected alert pause state: '{pause_state}' (expected 'on' or 'off')")
                
        except Exception as e:
            self.log(f"❌ Error checking alert pause state: {e}")
            # Continue with analysis if we can't read the state
        
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
        
        # AI recommendation with enhanced error handling
        if self.ai_enabled:
            alert_context.recommendation = self.get_ai_recommendation(alert_context)
        else:
            alert_context.recommendation = "AI disabled - check API key configuration"
        
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
            
            if lights_on is None or lights_off is None:
                self.log(f"⚠️ Missing lights schedule entities: ON={lights_on}, OFF={lights_off}")
                return True
            
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
            self.log(f"❌ Error determining day period: {e}")
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
                except (ValueError, TypeError) as e:
                    self.log(f"❌ Error getting threshold {threshold_key} for {sensor_name}: {e}")
        
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
        """Send notifications based on severity with enhanced error handling"""
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
            service_name = self.mobile_notify.replace("notify.", "")
            self.call_service(f"notify/{service_name}", 
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
            self.log(f"📱 Mobile notification sent successfully")
        except Exception as e:
            self.log(f"❌ Error sending mobile notification: {e}")
        
        # TTS for critical alerts
        if context.severity == "CRITICAL":
            try:
                tts_message = f"Critical grow room alert. {context.sensor_name} is {context.current_value:.1f} {unit}."
                self.call_service("tts/cloud_say",
                                entity_id=self.tts_entity,
                                message=tts_message)
                self.log(f"🔊 TTS alert sent successfully")
            except Exception as e:
                self.log(f"❌ Error sending TTS: {e}")
        
        self.log(f"🚨 Alert sent: {context.sensor_name} {context.severity} - {context.current_value}{unit}")

    def get_ai_recommendation(self, context: AlertContext) -> str:
        """Enhanced AI recommendations with better error handling"""
        if not self.ai_enabled:
            self.log("🤖 AI disabled - no API key")
            return "AI unavailable - check API key configuration"
        
        try:
            # Build context from Python data structures
            sensor_context = {}
            for name, data in self.sensor_data.items():
                if len(data['history']) > 0:
                    recent_values = [r.value for r in list(data['history'])[-10:]]
                    sensor_context[name] = {
                        'current': recent_values[-1] if recent_values else None,
                        'trend': self.calculate_trend(name),
                        'recent_readings': recent_values
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
            
            self.log("🤖 Calling OpenAI API for recommendation...")
            response = requests.post('https://api.openai.com/v1/chat/completions',
                                   headers=headers, json=data, timeout=10)
            
            if response.status_code == 200:
                result = response.json()
                recommendation = result['choices'][0]['message']['content'].strip()
                self.log(f"✅ AI recommendation received: {recommendation[:50]}...")
                return recommendation
            else:
                error_msg = f"API error: {response.status_code}"
                if response.text:
                    error_msg += f" - {response.text[:100]}"
                self.log(f"❌ AI API error: {error_msg}")
                return "AI temporarily unavailable - API error"
                
        except requests.RequestException as e:
            self.log(f"❌ Network error calling AI API: {e}")
            return "AI temporarily unavailable - network error"
        except Exception as e:
            self.log(f"❌ Unexpected error getting AI recommendation: {e}")
            return "AI error occurred - check logs"

    def ai_trend_analysis(self, kwargs=None):
        """Periodic AI analysis of trends and patterns with enhanced error handling"""
        if not self.ai_enabled:
            self.log("🤖 AI trend analysis skipped - AI disabled")
            return
        
        try:
            # Analyze patterns across all sensors
            patterns = {}
            for sensor_name, data in self.sensor_data.items():
                if len(data['history']) >= 30:  # At least 5 minutes of data
                    values = [r.value for r in list(data['history'])[-30:]]
                    patterns[sensor_name] = {
                        'mean': statistics.mean(values),
                        'stdev': statistics.stdev(values) if len(values) > 1 else 0,
                        'trend': self.calculate_trend(sensor_name),
                        'recent_violations': data['violation_count']
                    }
            
            # Only analyze if we have significant patterns
            if len(patterns) < 3:
                self.log("🔍 AI trend analysis skipped - insufficient data")
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
                
                self.log(f"📊 AI Trend Summary: {summary}")
            else:
                self.log("✅ AI trend analysis - no concerning patterns detected")
                
        except Exception as e:
            self.log(f"❌ Error in AI trend analysis: {e}")

    def save_persistent_data(self, kwargs=None):
        """Save data to survive HA restarts"""
        try:
            # Convert deques to lists for pickling
            save_data = {
                'sensor_data': {},
                'system_stats': self.system_stats,
                'last_save': datetime.now()
            }
            
            for sensor_name, data in self.sensor_data.items():
                save_data['sensor_data'][sensor_name] = {
                    'violation_count': data['violation_count'],
                    'last_alert_time': data['last_alert_time'],
                    'last_notified_value': data['last_notified_value'],
                    'previous_value': data['previous_value']
                    # Don't save full history - too large
                }
            
            with open(self.data_file, 'wb') as f:
                pickle.dump(save_data, f)
                
        except Exception as e:
            self.log(f"❌ Error saving persistent data: {e}")

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
            self.log(f"❌ Error loading persistent data: {e}")

    def system_health_check(self, kwargs=None):
        """Periodic system health reporting"""
        uptime = datetime.now() - self.system_stats['start_time']
        
        health_report = {
            'uptime_hours': uptime.total_seconds() / 3600,
            'total_alerts': self.system_stats['total_alerts'],
            'alerts_by_sensor': self.system_stats['alerts_by_sensor'],
            'sensors_active': len([s for s in self.sensor_data.values() if len(s['history']) > 0])
        }
        
        self.log(f"🏥 System Health: {health_report}")
