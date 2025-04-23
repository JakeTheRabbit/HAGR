# Improved Crop Steering System - Installation Guide

This guide explains how to install and configure the improved crop steering system for Home Assistant. The improvements address several issues with the original system, making it more reliable and robust.

## What's Fixed

1. **Time Calculation Issues**: Fixed improper date manipulation that could cause errors on the 1st day of the month.
2. **Configuration Access**: Converted YAML dictionary values to proper Home Assistant entities for reliable access in templates.
3. **Historical Data Access**: Replaced unsupported history method with statistics sensors.
4. **Time Matching**: Improved time trigger reliability by using time windows instead of exact matching.
5. **Code Efficiency**: Simplified complex templates and improved error handling.

## Installation Steps

### Step 1: Copy the Improved Files

Copy all the improved files to your Home Assistant configuration directory:

- `crop_steering_config_entities.yaml`
- `crop_steering_improved_sensors.yaml`
- `crop_steering_improved_automations.yaml`
- `crop_steering_improved_package.yaml`
- `crop_steering_dryback_tracking.yaml`
- `crop_steering_zone_controls.yaml`
- `crop_steering_variables.yaml`

### Step 2: Update Your Configuration

Add the improved package to your Home Assistant configuration by adding the following to your `configuration.yaml`:

```yaml
homeassistant:
  packages:
    crop_steering: !include crop_steering_improved_package.yaml
```

### Step 3: Configure Your Sensors

Edit the `crop_steering_variables.yaml` file to match your sensor entities:

```yaml
sensors:
  # Zone 1 sensors
  zone_1:
    vwc_front: sensor.YOUR_ZONE1_FRONT_VWC_SENSOR
    vwc_back: sensor.YOUR_ZONE1_BACK_VWC_SENSOR
    ec_front: sensor.YOUR_ZONE1_FRONT_EC_SENSOR
    ec_back: sensor.YOUR_ZONE1_BACK_EC_SENSOR
    temp: sensor.YOUR_ZONE1_TEMP_SENSOR
```

Repeat for all zones you have.

### Step 4: Configure Irrigation Control

Set up your irrigation valves and pump in the same file:

```yaml
irrigation:
  # Main irrigation pump/switch entity
  pump: switch.YOUR_IRRIGATION_PUMP
  
  # Valve entities
  valves:
    mainline: switch.YOUR_MAINLINE_VALVE
    zone_1: switch.YOUR_ZONE1_VALVE
    zone_2: switch.YOUR_ZONE2_VALVE
    zone_3: switch.YOUR_ZONE3_VALVE
    waste: switch.YOUR_WASTE_VALVE
```

### Step 5: Restart Home Assistant

After making all the changes, restart Home Assistant to apply them.

## Configuration Entities

The improved system uses input helpers (entities) instead of YAML dictionaries for configuration values. These entities are defined in `crop_steering_config_entities.yaml` and include:

- EC targets for each phase and growth mode
- Irrigation settings for each phase
- Substrate settings
- Dryback targets

You can adjust these values through the Home Assistant UI or by editing the configuration file.

## Understanding the Improvements

### Time Calculation Fix

The original code used direct day manipulation which could fail on the 1st day of the month:

```yaml
# Original (problematic)
{% set lights_on_dt = lights_on_dt.replace(day=lights_on_dt.day-1) %}

# Improved (reliable)
{% set lights_on_dt = lights_on_dt - timedelta(days=1) %}
```

### Configuration Access Fix

The original code tried to access YAML dictionary values directly, which doesn't work in Home Assistant:

```yaml
# Original (problematic)
{{ states('irrigation_settings.p3.veg_last_irrigation') }}

# Improved (reliable)
{{ states('input_number.p3_veg_last_irrigation') }}
```

### Historical Data Access Fix

The original code used an unsupported `.history()` method:

```yaml
# Original (problematic)
{% set history = states.sensor.avg_vwc.history(1, false) | default([]) %}

# Improved (reliable)
# Added statistics sensor
- platform: statistics
  name: "VWC Statistics"
  entity_id: sensor.avg_vwc
  sampling_size: 10
  max_age:
    minutes: 10

# Then used it in templates
{% set previous = states('sensor.vwc_statistics_mean') | float(0) %}
```

### Time Matching Fix

The original code used exact string matching for time triggers:

```yaml
# Original (problematic)
value_template: >
  {% set current_time = now().strftime('%H:%M:%S') %}
  {% set p3_start_time = states('sensor.p3_start_time_calculated') %}
  {{ current_time == p3_start_time }}

# Improved (reliable)
value_template: >
  {% set current_timestamp = as_timestamp(now()) %}
  {% set p3_start = as_timestamp(today_at(states('sensor.p3_start_time_calculated'))) %}
  {% set diff_seconds = current_timestamp - p3_start %}
  {{ diff_seconds >= 0 and diff_seconds < 60 }}
```

## Troubleshooting

### Common Issues

#### Entity Not Found Errors

If you see errors about entities not being found, check that:

1. You've included all the required files in your configuration
2. You've correctly configured your sensor entities in `crop_steering_variables.yaml`
3. You've restarted Home Assistant after making changes

#### Irrigation Not Working

If irrigation isn't working as expected:

1. Check that your pump and valve entities are correctly configured
2. Verify that the irrigation automations are enabled
3. Check the Home Assistant logs for any errors
4. Test your valves and pump manually to ensure they work

#### Phase Transitions Not Working

If phase transitions aren't happening automatically:

1. Check that your sunrise and sunset times are correctly set
2. Verify that the phase transition automations are enabled
3. Check the Home Assistant logs for any errors

## Advanced Customization

### Adding More Zones

To add more than 3 zones:

1. Edit `crop_steering_variables.yaml` to add new zone sensors
2. Edit `crop_steering_zone_controls.yaml` to add new zone valves
3. Add new zone-specific template sensors if needed

### Custom Irrigation Strategies

To implement custom irrigation strategies:

1. Edit the phase-specific automations in `crop_steering_improved_automations.yaml`
2. Modify the trigger conditions and actions
3. Add new input helpers for custom parameters if needed

## Support

If you encounter any issues with the improved crop steering system, please check:

1. The Home Assistant logs for any errors
2. That all required files are included in your configuration
3. That all sensor and switch entities are correctly configured

For further assistance, please refer to the Home Assistant community forums or the project's GitHub repository.
