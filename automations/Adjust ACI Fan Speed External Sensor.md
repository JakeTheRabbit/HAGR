# Home Assistant Fan Speed Adjustment Automation

This automation adjusts the fan speed based on the current temperature and target temperature (day or night). The logic ensures that the fan speed increases or decreases based on the difference between the current and target temperatures.

## Required Entities

1. **Current Temperature Sensor:** `sensor.temp` any temperature sensor
2. **Day Temperature Target:** `input_number.day_temp`
3. **Night Temperature Target:** `input_number.night_temp`
4. **Fan Speed Control:** `number.fan_speed` this is for AC Infinity fans 0-10
5. **Lights On Time:** `input_datetime.lights_on`
6. **Lights Off Time:** `input_datetime.lights_off`

## Logic Explanation

The automation triggers every 5 minutes or when the temperature or target temperature changes. It checks whether the current time is during the day or night and adjusts the fan speed accordingly. The fan speed is adjusted to ensure the temperature stays within 1 degree of the target temperature.

## Helpers: Add these to your configuaration.yaml file before creating the automation or script. You can use your own.

```
# Input Number: Day Temperature Target
input_number:
  day_temp:
    name: "Day Temperature Target"
    min: 0
    max: 50
    step: 0.5
    unit_of_measurement: "°C"
    icon: "mdi:thermometer"

# Input Number: Night Temperature Target
  night_temp:
    name: "Night Temperature Target"
    min: 0
    max: 50
    step: 0.5
    unit_of_measurement: "°C"
    icon: "mdi:thermometer"

# Input Datetime: Lights On Time
input_datetime:
  lights_on:
    name: "Lights On Time"
    has_date: false
    has_time: true
    icon: "mdi:lightbulb-on"

# Input Datetime: Lights Off Time
  lights_off:
    name: "Lights Off Time"
    has_date: false
    has_time: true
    icon: "mdi:lightbulb-off"
```

## Automation: Adjust Fan Speed Based on Temperature
Add this to automations through the gui. 

```yaml
alias: Adjust Fan Speed Based on Temperature
description: Adjusts fan speed based on temperature and time of day
trigger:
  - platform: state
    entity_id: sensor.temp
  - platform: time_pattern
    minutes: "/2"  # Adjusts every 5 minutes change this to whatever you want this determines how often the reading from the temperature sensor is taken.
  - platform: state
    entity_id: input_number.day_temp
  - platform: state
    entity_id: input_number.night_temp
condition: []
action:
  - choose:
      - conditions:
          - condition: time
            after: input_datetime.lights_on
            before: input_datetime.lights_off
        sequence:
          - service: script.adjust_fan_speed
            data:
              current_temp_entity: sensor.temp
              target_temp_entity: input_number.day_temp
              fan_speed_entity: number.fan_speed
      - conditions:
          - condition: or
            conditions:
              - condition: time
                before: input_datetime.lights_on
              - condition: time
                after: input_datetime.lights_off
        sequence:
          - service: script.adjust_fan_speed
            data:
              current_temp_entity: sensor.temp
              target_temp_entity: input_number.night_temp
              fan_speed_entity: number.fan_speed
mode: single
```

## Script: Adjust Fan Speed

Add this to the scripts through the gui.

```yaml
alias: Adjust Fan Speed
sequence:
  - variables:
      current_temp_entity: "{{ current_temp_entity | default('sensor.temp') }}"
      target_temp_entity: "{{ target_temp_entity | default('input_number.day_temp') }}"
      fan_speed_entity: "{{ fan_speed_entity | default('number.fan_speed') }}"
      current_temp: "{{ states(current_temp_entity) | float(default=0) }}"
      target_temp: "{{ states(target_temp_entity) | float(default=0) }}"
      current_speed: "{{ states(fan_speed_entity) | float(default=0) }}"
  - choose:
      - conditions:
          - condition: template
            value_template: >
              {{ current_temp not in ['unknown', 'unavailable'] and target_temp not in ['unknown', 'unavailable'] }}
        sequence:
          - variables:
              current_temp: "{{ current_temp | float }}"
              target_temp: "{{ target_temp | float }}"
              new_speed: >
                {% if current_temp > target_temp + 1 %}
                  {{ [current_speed + 1, 10] | min }}  # Increase speed, max 10
                {% elif current_temp < target_temp - 1 %}
                  {{ [current_speed - 1, 0] | max }}  # Decrease speed, min 0
                {% else %}
                  {{ current_speed }}  # Maintain current speed
                {% endif %}
          - service: number.set_value
            target:
              entity_id: "{{ fan_speed_entity }}"
            data:
              value: "{{ new_speed }}"
          - delay: '00:01:00'  # Add a 1 minute delay between adjustments - change this to whatever, this determines the minimum time to wait between adjusting the fan regardles of the time interval in the automation. 
      - conditions:
          - condition: template
            value_template: >
              {{ current_temp in ['unknown', 'unavailable'] or target_temp in ['unknown', 'unavailable'] }}
        sequence:
          - service: number.set_value
            target:
              entity_id: "{{ fan_speed_entity }}"
            data:
              value: "{{ current_speed }}"
fields:
  current_temp_entity:
    description: "The entity_id of the current temperature sensor"
    example: "sensor.temp"
  target_temp_entity:
    description: "The entity_id of the target temperature (day or night)"
    example: "input_number.day_temp"
  fan_speed_entity:
    description: "The entity_id of the fan speed control"
    example: "number.fan_speed"
mode: single
```


