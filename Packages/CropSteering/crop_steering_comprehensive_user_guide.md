# Advanced Crop Steering System for Home Assistant

## Comprehensive User Guide

---

## Table of Contents

1. [System Overview](#system-overview)
2. [Installation](#installation)
3. [Core Features](#core-features)
4. [Configuration Guide](#configuration-guide)
5. [Dashboard & Visualization](#dashboard--visualization)
6. [Troubleshooting](#troubleshooting)
7. [Advanced Usage](#advanced-usage)
8. [Technical Reference](#technical-reference)

---

## System Overview

The Advanced Crop Steering System is a sophisticated irrigation and nutrient management solution for indoor growing environments, implemented as a Home Assistant integration. It provides precise control over irrigation timing, duration, and nutrient delivery based on real-time substrate conditions.

### The Four-Phase Approach

The system implements a scientifically-backed four-phase approach to crop steering:

1. **P0: Pre-Irrigation Dry Back** - Allows the substrate to dry down after lights-on, encouraging root growth and oxygen uptake
2. **P1: Ramp-Up Phase** - Gradually increases irrigation shot sizes to bring the substrate to optimal moisture levels
3. **P2: Maintenance Phase** - Maintains optimal moisture levels with regular irrigation, adjusting based on EC levels
4. **P3: Overnight Dry Back** - Reduces irrigation before lights-off to create a controlled dry-down period

### Key Benefits

- **Precision Control**: Automate irrigation based on exact substrate conditions
- **Growth Steering**: Switch between vegetative and generative growth modes
- **Automatic Dryback Tracking**: Automatically detect and measure dryback cycles
- **EC Management**: Adjust irrigation based on EC levels to maintain optimal nutrient balance
- **Multi-Zone Support**: Control up to 3 irrigation zones independently
- **Historical Data**: Track and analyze dryback patterns over time

---

## Installation

### Prerequisites

- Home Assistant installation (Core, OS, Container, or Supervised)
- Substrate moisture sensors (VWC) for each zone
- EC sensors for each zone (optional but recommended)
- Irrigation system with controllable valves
- Temperature sensors (optional)

### Installation Steps

1. **Copy Files**: Copy all the following files to your Home Assistant configuration directory:

   ```
   crop_steering_improved_package.yaml
   crop_steering_variables_fixed.yaml
   crop_steering_config_entities.yaml
   crop_steering_improved_sensors.yaml
   crop_steering_improved_automations.yaml
   crop_steering_dryback_tracking.yaml
   crop_steering_zone_controls.yaml
   ```

2. **Update Configuration**: Add the following to your `configuration.yaml`:

   ```yaml
   homeassistant:
     packages:
       crop_steering_variables: !include packages/CropSteering/crop_steering_variables_fixed.yaml
       crop_steering_config: !include packages/CropSteering/crop_steering_config_entities.yaml
       crop_steering_dryback: !include packages/CropSteering/crop_steering_dryback_tracking.yaml
       crop_steering_automations: !include packages/CropSteering/crop_steering_improved_automations.yaml
       crop_steering_sensors: !include packages/CropSteering/crop_steering_improved_sensors.yaml
       crop_steering_zones: !include packages/CropSteering/crop_steering_zone_controls.yaml
   ```

3. **Configure Sensors**: Edit the `crop_steering_variables_fixed.yaml` file to match your sensor entities:

   ```yaml
   # Zone 1 sensors
   zone1_vwc_front_entity:
     name: Zone 1 VWC Front Sensor
     initial: sensor.YOUR_ZONE1_FRONT_VWC_SENSOR
   zone1_vwc_back_entity:
     name: Zone 1 VWC Back Sensor
     initial: sensor.YOUR_ZONE1_BACK_VWC_SENSOR
   ```

4. **Configure Irrigation Control**: Set up your irrigation valves and pump in the same file:

   ```yaml
   irrigation_pump_entity:
     name: Irrigation Pump Entity
     initial: switch.YOUR_IRRIGATION_PUMP
   irrigation_valve_mainline_entity:
     name: Mainline Valve Entity
     initial: switch.YOUR_MAINLINE_VALVE
   ```

5. **Restart Home Assistant**: After making all the changes, restart Home Assistant to apply them.

---

## Core Features

### Four-Phase Irrigation Control

The system automatically transitions between four distinct irrigation phases throughout the day:

#### P0: Pre-Irrigation Dry Back

- **Purpose**: Allow substrate to dry down after lights-on
- **Starts**: Automatically at lights-on
- **Ends**: When either:
  - Target dryback percentage is reached (configurable)
  - Maximum wait time is reached (configurable)
- **Benefits**: Encourages root growth, increases oxygen in the root zone

#### P1: Ramp-Up Phase

- **Purpose**: Gradually increase irrigation to reach optimal moisture
- **Mechanism**: Delivers progressively larger irrigation shots at regular intervals
- **Shot Sizing**: Starts with a small initial shot size, then increases by a configurable increment with each shot
- **Ends**: When either:
  - Maximum shot count is reached
  - Target VWC is reached
  - Target EC is reached

#### P2: Maintenance Phase

- **Purpose**: Maintain optimal moisture levels throughout the day
- **Mechanism**: Irrigates when VWC drops below a threshold
- **EC Adjustment**: Automatically adjusts VWC threshold based on EC levels:
  - If EC is too high: Irrigate sooner (higher VWC threshold)
  - If EC is too low: Delay irrigation (lower VWC threshold)
- **Zone-Specific**: Can irrigate individual zones based on their specific VWC readings

#### P3: Overnight Dry Back

- **Purpose**: Create a controlled dry-down period before lights-off
- **Starts**: Calculated time before lights-off (configurable)
- **Emergency Irrigation**: Will still irrigate if VWC drops below critical threshold
- **Benefits**: Encourages generative growth, prevents overnight saturation

### Automatic Dryback Detection and Tracking

The system automatically identifies and tracks dryback cycles:

- **Peak Detection**: Automatically identifies when VWC reaches a peak
- **Valley Detection**: Automatically identifies when VWC reaches a valley
- **Dryback Calculation**: Calculates percentage and duration of each dryback cycle
- **Historical Tracking**: Stores dryback history for analysis
- **Statistics**: Calculates average dryback percentage and duration over 24 hours

### EC Management

The system adjusts irrigation based on EC (Electrical Conductivity) levels:

- **EC Targets**: Configurable EC targets for each phase and growth mode
- **EC Ratio**: Calculates ratio of current EC to target EC
- **Threshold Adjustment**: Adjusts VWC thresholds based on EC ratio
- **Phase-Specific Targets**: Different EC targets for each phase and growth mode

### Multi-Zone Support

The system supports up to 3 irrigation zones with independent control:

- **Zone Selection**: Choose which zones to irrigate (All, individual, or combinations)
- **Zone-Specific Monitoring**: Track VWC and EC for each zone independently
- **Zone-Specific Dryback**: Calculate dryback percentage for each zone
- **Automatic Zone Selection**: Automatically select zones that need irrigation

### Growth Mode Switching

The system supports two growth modes:

- **Vegetative**: Focuses on vegetative growth with more frequent irrigation
- **Generative**: Focuses on generative growth with more pronounced dryback

Switching between modes automatically adjusts:
- Dryback targets
- EC targets
- P3 start time
- Irrigation frequency

### Sensor Aggregation

The system aggregates readings from multiple sensors:

- **VWC Aggregation**: Combines readings from front and back sensors for each zone
- **EC Aggregation**: Combines EC readings from front and back sensors
- **Aggregation Methods**: Choose between average, min, or max aggregation
- **Validation**: Filters out invalid sensor readings based on configurable thresholds

### Historical Data Tracking

The system tracks historical data for analysis:

- **Dryback History**: Stores details of each dryback cycle
- **24-Hour Statistics**: Calculates average dryback percentage and duration over 24 hours
- **Dryback Count**: Tracks number of dryback cycles in the last 24 hours
- **Data Retention**: Configurable data retention period

---

## Configuration Guide

### Sensor Entity Mappings

Configure your sensor entities in the `crop_steering_variables_fixed.yaml` file:

| Setting | Description | Default |
|---------|-------------|---------|
| `zone1_vwc_front_entity` | Entity ID for Zone 1 front VWC sensor | `sensor.vwc_r1_front` |
| `zone1_vwc_back_entity` | Entity ID for Zone 1 back VWC sensor | `sensor.vwc_r1_back` |
| `zone1_ec_front_entity` | Entity ID for Zone 1 front EC sensor | `sensor.ec_r1_front` |
| `zone1_ec_back_entity` | Entity ID for Zone 1 back EC sensor | `sensor.ec_r1_back` |
| `zone1_temp_entity` | Entity ID for Zone 1 temperature sensor | `sensor.temp_r1` |

Similar settings exist for Zone 2 and Zone 3.

### Irrigation Control Entities

Configure your irrigation control entities:

| Setting | Description | Default |
|---------|-------------|---------|
| `irrigation_pump_entity` | Entity ID for irrigation pump | `switch.irrigation_pump` |
| `irrigation_valve_mainline_entity` | Entity ID for mainline valve | `switch.espoe_irrigation_relay_1_2` |
| `irrigation_valve_zone1_entity` | Entity ID for Zone 1 valve | `switch.f1_irrigation_relays_relay_1` |
| `irrigation_valve_zone2_entity` | Entity ID for Zone 2 valve | `switch.f1_irrigation_relays_relay_3` |
| `irrigation_valve_zone3_entity` | Entity ID for Zone 3 valve | `switch.f1_irrigation_relays_relay_2` |
| `irrigation_valve_waste_entity` | Entity ID for waste valve | `switch.f1_irrigation_relays_relay_4` |

### Dryback Detection Settings

Configure dryback detection parameters:

| Setting | Description | Default | Range |
|---------|-------------|---------|-------|
| `cs_dryback_peak_detection_threshold` | Threshold for detecting VWC peaks | 0.5% | 0.1-5.0% |
| `cs_dryback_valley_detection_threshold` | Threshold for detecting VWC valleys | 0.5% | 0.1-5.0% |
| `cs_dryback_min_duration` | Minimum duration to consider a valid dryback | 60 min | 10-1440 min |
| `cs_dryback_min_percentage` | Minimum percentage to consider a valid dryback | 5% | 1-50% |

### Irrigation Settings

#### General Settings

| Setting | Description | Default | Range |
|---------|-------------|---------|-------|
| `cs_dripper_flow_rate` | Flow rate of each dripper | 1.2 L/hr | 0.1-10.0 L/hr |
| `cs_substrate_volume` | Volume of substrate per plant | 3.5 L | 0.1-20.0 L |

#### P0 Phase Settings

| Setting | Description | Default | Range |
|---------|-------------|---------|-------|
| `cs_p0_veg_dryback_target` | Target dryback percentage in vegetative mode | 2% | 0-20% |
| `cs_p0_gen_dryback_target` | Target dryback percentage in generative mode | 5% | 0-20% |
| `cs_p0_min_wait_time` | Minimum wait time before transitioning to P1 | 30 min | 0-300 min |
| `cs_p0_max_wait_time` | Maximum wait time before transitioning to P1 | 120 min | 0-300 min |

#### P1 Phase Settings

| Setting | Description | Default | Range |
|---------|-------------|---------|-------|
| `cs_p1_initial_shot_size_percent` | Initial irrigation shot size | 2% | 0-20% |
| `cs_p1_shot_size_increment_percent` | Increment for each subsequent shot | 1% | 0-10% |
| `cs_p1_time_between_shots` | Time between shots | 15 min | 1-60 min |
| `cs_p1_target_vwc` | Target VWC to reach | 30% | 10-50% |
| `cs_p1_max_shots` | Maximum number of shots | 6 | 1-20 |
| `cs_p1_max_shot_size_percent` | Maximum shot size | 10% | 1-30% |

#### P2 Phase Settings

| Setting | Description | Default | Range |
|---------|-------------|---------|-------|
| `cs_p2_shot_size_percent` | Irrigation shot size | 5% | 1-30% |
| `cs_p2_veg_frequency` | Minimum time between irrigations in vegetative mode | 60 min | 10-300 min |
| `cs_p2_gen_frequency` | Minimum time between irrigations in generative mode | 120 min | 10-300 min |
| `cs_p2_vwc_threshold` | VWC threshold to trigger irrigation | 25% | 10-50% |
| `cs_p2_ec_high_threshold` | EC ratio threshold for high EC adjustment | 1.2 | 1.0-2.0 |
| `cs_p2_ec_low_threshold` | EC ratio threshold for low EC adjustment | 0.8 | 0.5-1.0 |
| `cs_p2_vwc_adjustment_high_ec` | VWC threshold adjustment for high EC | 2% | -5-5% |
| `cs_p2_vwc_adjustment_low_ec` | VWC threshold adjustment for low EC | -2% | -5-5% |

#### P3 Phase Settings

| Setting | Description | Default | Range |
|---------|-------------|---------|-------|
| `cs_p3_veg_last_irrigation` | Minutes before lights-off for last irrigation in vegetative mode | 60 min | 0-300 min |
| `cs_p3_gen_last_irrigation` | Minutes before lights-off for last irrigation in generative mode | 180 min | 0-300 min |
| `cs_p3_emergency_vwc_threshold` | VWC threshold for emergency irrigation | 15% | 5-30% |
| `cs_p3_emergency_shot_size_percent` | Shot size for emergency irrigation | 3% | 1-20% |

### EC Target Settings

#### Vegetative Mode EC Targets

| Setting | Description | Default | Range |
|---------|-------------|---------|-------|
| `cs_ec_target_veg_p0` | EC target for P0 in vegetative mode | 1.6 mS/cm | 0.5-5.0 mS/cm |
| `cs_ec_target_veg_p1` | EC target for P1 in vegetative mode | 1.8 mS/cm | 0.5-5.0 mS/cm |
| `cs_ec_target_veg_p2` | EC target for P2 in vegetative mode | 2.0 mS/cm | 0.5-5.0 mS/cm |
| `cs_ec_target_veg_p3` | EC target for P3 in vegetative mode | 1.8 mS/cm | 0.5-5.0 mS/cm |

#### Generative Mode EC Targets

| Setting | Description | Default | Range |
|---------|-------------|---------|-------|
| `cs_ec_target_gen_p0` | EC target for P0 in generative mode | 1.8 mS/cm | 0.5-5.0 mS/cm |
| `cs_ec_target_gen_p1` | EC target for P1 in generative mode | 2.2 mS/cm | 0.5-5.0 mS/cm |
| `cs_ec_target_gen_p2` | EC target for P2 in generative mode | 2.5 mS/cm | 0.5-5.0 mS/cm |
| `cs_ec_target_gen_p3` | EC target for P3 in generative mode | 2.8 mS/cm | 0.5-5.0 mS/cm |
| `cs_ec_target_flush` | EC target for flushing | 0.8 mS/cm | 0.1-2.0 mS/cm |

### Light Schedule Settings

| Setting | Description | Default |
|---------|-------------|---------|
| `cs_lights_on_time` | Time when lights turn on | 06:00:00 |
| `cs_lights_off_time` | Time when lights turn off | 00:00:00 |
| `cs_lights_fade_in_minutes` | Duration of light fade-in | 30 min |
| `cs_lights_fade_out_minutes` | Duration of light fade-out | 30 min |

### Substrate Settings

| Setting | Description | Default | Range |
|---------|-------------|---------|-------|
| `cs_substrate_size` | Size of substrate | 6 in | 1-24 in |
| `cs_substrate_volume` | Volume of substrate | 3.5 L | 0.1-20 L |
| `cs_substrate_field_capacity` | Field capacity of substrate | 35% | 10-100% |
| `cs_substrate_saturation_point` | Saturation point of substrate | 45% | 10-100% |
| `cs_substrate_critical_vwc` | Critical VWC below which plant stress occurs | 10% | 5-30% |
| `cs_substrate_max_ec` | Maximum safe EC for substrate | 3.5 mS/cm | 1.0-10.0 mS/cm |
| `cs_substrate_water_retention_factor` | Water retention factor of substrate | 0.85 | 0.1-1.0 |

### Sensor Aggregation Settings

| Setting | Description | Default | Range |
|---------|-------------|---------|-------|
| `cs_aggregation_method` | Method for aggregating sensor readings | average | average, min, max |
| `cs_min_valid_vwc` | Minimum valid VWC reading | 1.0% | 0-50% |
| `cs_max_valid_vwc` | Maximum valid VWC reading | 80.0% | 10-100% |
| `cs_min_valid_ec` | Minimum valid EC reading | 0.1 mS/cm | 0-2.0 mS/cm |
| `cs_max_valid_ec` | Maximum valid EC reading | 5.0 mS/cm | 1.0-10.0 mS/cm |

---

## Dashboard & Visualization

The system includes several dashboard card examples that you can add to your Lovelace UI.

### Current Status Card

```yaml
type: entities
title: Crop Steering Status
entities:
  - entity: input_select.crop_steering_phase
  - entity: sensor.current_phase_description
  - entity: sensor.irrigation_status
  - entity: sensor.current_ec_target
  - entity: sensor.ec_ratio
```

### Dryback Tracking Cards

```yaml
type: entities
title: Current Dryback Status
entities:
  - entity: sensor.dryback_in_progress
  - entity: sensor.dryback_current_percentage
  - entity: sensor.dryback_current_duration
  - entity: sensor.dryback_last_peak_vwc
  - entity: sensor.avg_vwc
```

```yaml
type: gauge
title: Current Dryback Progress
entity: sensor.dryback_current_percentage
min: 0
max: 30
severity:
  green: 0
  yellow: 15
  red: 25
```

### VWC Monitoring Graph

```yaml
type: custom:apexcharts-card
header:
  title: VWC Monitoring
  show: true
graph_span: 24h
series:
  - entity: sensor.zone_1_vwc
    name: Zone 1 VWC
  - entity: sensor.zone_2_vwc
    name: Zone 2 VWC
  - entity: sensor.zone_3_vwc
    name: Zone 3 VWC
  - entity: sensor.dynamic_p2_dryback
    name: Dryback Threshold
    stroke_width: 1
    curve: stepline
    color: red
```

### Zone Control Card

```yaml
type: entities
title: Irrigation Zone Control
entities:
  - entity: input_select.active_irrigation_zones
  - entity: input_boolean.zone_1_enabled
  - entity: input_boolean.zone_2_enabled
  - entity: input_boolean.zone_3_enabled
  - entity: sensor.active_zones_count
```

### Configuration Card

```yaml
type: entities
title: Crop Steering Configuration
entities:
  - entity: input_select.cs_steering_mode
  - entity: input_number.cs_dripper_flow_rate
  - entity: input_number.cs_substrate_volume
  - entity: input_number.cs_p2_vwc_threshold
```

---

## Troubleshooting

### Common Issues

#### Entities Not Found

If you see errors about entities not being found:

1. Check that you've included all the required files in your configuration
2. Verify that you've correctly configured your sensor entities in `crop_steering_variables_fixed.yaml`
3. Restart Home Assistant after making changes

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

#### Dryback Detection Issues

If dryback detection isn't working properly:

1. Check that your VWC sensors are working correctly
2. Adjust the peak and valley detection thresholds
3. Ensure your VWC sensors are properly calibrated
4. Check that the dryback tracking automations are enabled

#### EC Management Issues

If EC management isn't working properly:

1. Check that your EC sensors are working correctly
2. Verify that the EC targets are correctly configured
3. Check the EC ratio calculation
4. Ensure your EC sensors are properly calibrated

### Diagnostic Tools

The system includes several diagnostic sensors that can help troubleshoot issues:

- `sensor.irrigation_status`: Shows the current status of irrigation
- `sensor.current_phase_description`: Shows a detailed description of the current phase
- `sensor.ec_ratio`: Shows the ratio of current EC to target EC
- `sensor.dryback_in_progress`: Shows whether a dryback cycle is in progress
- `sensor.dryback_potential_peak`: Shows the potential VWC peak value
- `sensor.dryback_potential_valley`: Shows the potential VWC valley value

---

## Advanced Usage

### Adding More Zones

To add more than 3 zones:

1. Edit `crop_steering_variables_fixed.yaml` to add new zone sensors:

   ```yaml
   zone4_vwc_front_entity:
     name: Zone 4 VWC Front Sensor
     initial: sensor.vwc_r4_front
   zone4_vwc_back_entity:
     name: Zone 4 VWC Back Sensor
     initial: sensor.vwc_r4_back
   ```

2. Edit `crop_steering_zone_controls.yaml` to add new zone valves:

   ```yaml
   irrigation_valve_zone4_entity:
     name: Zone 4 Valve Entity
     initial: switch.f1_irrigation_relays_relay_5
   ```

3. Add new zone-specific template sensors:

   ```yaml
   zone_4_vwc:
     friendly_name: "Zone 4 VWC"
     value_template: >
       {% set front = states('sensor.vwc_r4_front') | float(0) %}
       {% set back = states('sensor.vwc_r4_back') | float(0) %}
       {% if front > 0 and back > 0 %}
         {{ ((front + back) / 2) | round(1) }}
       {% elif front > 0 %}
         {{ front | round(1) }}
       {% elif back > 0 %}
         {{ back | round(1) }}
       {% else %}
         0
       {% endif %}
     unit_of_measurement: "%"
     icon_template: "mdi:water-percent"
   ```

4. Update the zone selection input_select:

   ```yaml
   active_irrigation_zones:
     name: Active Irrigation Zones
     options:
       - All Zones
       - Zone 1 Only
       - Zone 2 Only
       - Zone 3 Only
       - Zone 4 Only
       # Add more combinations as needed
     initial: All Zones
     icon: mdi:sprinkler-variant
   ```

### Custom Irrigation Strategies

To implement custom irrigation strategies:

1. Edit the phase-specific automations in `crop_steering_improved_automations.yaml`
2. Modify the trigger conditions and actions
3. Add new input helpers for custom parameters if needed

For example, to add a custom irrigation strategy based on time of day:

```yaml
- id: custom_time_based_irrigation
  alias: Custom Time-Based Irrigation
  description: "Irrigates at specific times of day"
  trigger:
    - platform: time
      at: "10:00:00"
    - platform: time
      at: "14:00:00"
    - platform: time
      at: "18:00:00"
  condition:
    - condition: state
      entity_id: input_select.crop_steering_phase
      state: P2
    - condition: state
      entity_id: switch.irrigation_pump
      state: 'off'
  action:
    - service: switch.turn_on
      target:
        entity_id: switch.irrigation_pump
    - delay:
        seconds: "{{ states('sensor.p2_shot_duration_seconds') }}"
    - service: switch.turn_off
      target:
        entity_id: switch.irrigation_pump
```

### Integration with Other Systems

The crop steering system can be integrated with other Home Assistant components:

#### Nutrient Dosing Integration

```yaml
- id: adjust_ec_based_on_crop_phase
  alias: Adjust EC Based on Crop Phase
  trigger:
    - platform: state
      entity_id: input_select.crop_steering_phase
  action:
    - service: input_number.set_value
      target:
        entity_id: input_number.nutrient_ec_target
      data:
        value: "{{ states('sensor.current_ec_target') }}"
```

#### Climate Control Integration

```yaml
- id: adjust_vpd_based_on_crop_phase
  alias: Adjust VPD Based on Crop Phase
  trigger:
    - platform: state
      entity_id: input_select.crop_steering_phase
  action:
    - choose:
        - conditions:
            - condition: state
              entity_id: input_select.crop_steering_phase
              state: P0
          sequence:
            - service: input_number.set_value
              target:
                entity_id: input_number.target_vpd
              data:
                value: 1.0
        - conditions:
            - condition: state
              entity_id: input_select.crop_steering_phase
              state: P3
          sequence:
            - service: input_number.set_value
              target:
                entity_id: input_number.target_vpd
              data:
                value: 1.2
```

---

## Technical Reference

### File Structure

| File | Description |
|------|-------------|
| `crop_steering_improved_package.yaml` | Main package file that includes all other files |
| `crop_steering_variables_fixed.yaml` | Configuration variables and sensor mappings |
| `crop_steering_config_entities.yaml` | Configuration entities for the UI |
| `crop_steering_improved_sensors.yaml` | Template sensors for calculations |
| `crop_steering_improved_automations.yaml` | Automations for phase transitions and irrigation control |
| `crop_steering_dryback_tracking.yaml` | Dryback detection and tracking |
| `crop_steering_zone_controls.yaml` | Multi-zone control |

### Entity Reference

#### Input Selects

- `input_select.crop_steering_phase`: Current phase (P0, P1, P2, P3)
- `input_select.cs_steering_mode`: Growth mode (Vegetative, Generative)
- `input_select.active_irrigation_zones`: Active zones for irrigation
- `input_select.cs_aggregation_method`: Method for aggregating sensor readings

#### Sensors

- `sensor.avg_vwc`: Average VWC across all zones
- `sensor.min_vwc`: Minimum VWC across all zones
- `sensor.max_vwc`: Maximum VWC across all zones
- `sensor.avg_ec`: Average EC across all zones
- `sensor.current_ec_target`: Current EC target based on phase and mode
- `sensor.ec_ratio`: Ratio of current EC to target EC
- `sensor.minutes_since_lights_on`: Minutes since lights turned on
- `sensor.minutes_until_lights_off`: Minutes until lights turn off
- `sensor.p3_start_time_calculated`: Calculated start time for P3
- `sensor.dryback_in_progress`: Whether a dryback cycle is in progress
- `sensor.dryback_current_percentage`: Current dryback percentage
- `sensor.dryback_current_duration`: Current dryback duration
- `sensor.dryback_last_peak_time`: Time of last VWC peak
- `sensor.dryback_last_valley_time`: Time of last VWC valley
- `sensor.dryback_last_percentage`: Percentage of last completed dryback
- `sensor.dryback_last_duration`: Duration of last completed dryback
- `sensor.dryback_avg_percentage_24h`: Average dryback percentage over 24 hours
- `sensor.dryback_avg_duration_24h`: Average dryback duration over 24 hours
- `sensor.dryback_count_24h`: Number of dryback cycles in last 24 hours

#### Zone-Specific Sensors

- `sensor.zone_1_vwc`: Average VWC for Zone 1
- `sensor.zone_2_vwc`: Average VWC for Zone 2
- `sensor.zone_3_vwc`: Average VWC for Zone 3
- `sensor.zone_1_ec`: Average EC for Zone 1
- `sensor.zone_2_ec`: Average EC for Zone 2
- `sensor.zone_3_ec`: Average EC for Zone 3
- `sensor.zone_1_dryback_percentage`: Dryback percentage for Zone 1
- `sensor.zone_2_dryback_percentage`: Dryback percentage for Zone 2
- `sensor.zone_3_dryback_percentage`: Dryback percentage for Zone 3

#### Automations

- `automation.improved_crop_steering_change_to_p0`: Changes to P0 at lights-on
- `automation.improved_crop_steering_p0_to_p1`: Transitions from P0 to P1
- `automation.improved_crop_steering_p1_to_p2`: Transitions from P1 to P2
- `automation.improved_crop_steering_to_p3`: Transitions from P2 to P3
- `automation.improved_crop_steering_p1_irrigation`: Controls P1 irrigation
- `automation.improved_crop_steering_p2_irrigation_on`: Turns on P2 irrigation
- `automation.improved_crop_steering_p2_irrigation_off_timed`: Turns off P2 irrigation after duration
- `automation.improved_crop_steering_p2_irrigation_off_capacity`: Turns off P2 irrigation when field capacity is reached
- `automation.improved_crop_steering_p3_emergency_irrigation`: Provides emergency irrigation during P3
- `automation.improved_steering_mode_changed`: Updates parameters when steering mode changes
- `automation.update_zone_status_from_selector`: Updates zone status based on selector
- `automation.turn_on_irrigation_pump`: Turns on irrigation pump and valves
- `automation.turn_off_irrigation_pump`: Turns off irrigation pump and valves
- `automation.zone_specific_p2_irrigation`: Zone-specific P2 irrigation
- `automation.zone_specific_p3_irrigation`: Zone-specific P3 emergency irrigation
- `automation.dryback_completed`: Records completed dryback cycles
- `automation.dryback_reset_potential_peak`: Resets potential peak detection after irrigation

### Conclusion

The Advanced Crop Steering System provides a comprehensive solution for precision irrigation control in indoor growing environments. By leveraging Home Assistant's powerful automation capabilities, it enables growers to implement sophisticated irrigation strategies based on real-time substrate conditions.

The system's four-phase approach, automatic dryback tracking, EC management, and multi-zone support provide unparalleled control over the growing environment, allowing for precise steering between vegetative and generative growth modes.

With extensive configuration options and integration capabilities, the system can be tailored to meet the specific needs of any indoor growing operation, from small hobby grows to large commercial facilities.
