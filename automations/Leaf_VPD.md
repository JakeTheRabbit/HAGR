# Leaf VPD Control System for Home Assistant

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Home Assistant](https://img.shields.io/badge/Home%20Assistant-2023.8.0-blue.svg)](https://www.home-assistant.io/)

A comprehensive system for controlling Vapor Pressure Deficit (VPD) based on actual leaf temperature measurements in Home Assistant.

## Table of Contents
- [Why Leaf VPD vs Air VPD?](#why-leaf-vpd-vs-air-vpd)
- [How It Works](#how-it-works)
- [Target Values](#target-values)
- [Required Hardware](#required-hardware)
- [Configuration](#configuration)
- [Mathematical Logic](#mathematical-logic-explained)
- [Common Issues and Solutions](#common-issues-and-solutions)
- [Best Practices](#best-practices)

## Why Leaf VPD vs Air VPD?

Leaf VPD measures the vapor pressure difference between the leaf surface and surrounding air, while Air VPD only considers the ambient conditions. Leaf VPD is more accurate for plant health as it accounts for actual leaf temperature, which can be several degrees different from air temperature due to transpiration and radiation effects. This difference is crucial as it directly affects the plant's transpiration rate and nutrient uptake.

## How It Works

### 1. VPD Calculation
- Measures actual leaf temperature using thermal camera
- Calculates saturated vapor pressure at both leaf and air temperatures
- Determines target humidity needed to maintain desired leaf VPD

### 2. Control Logic
- Calculates required humidity for target Leaf VPD
- Maintains humidity within specified tolerance range
- Controls dehumidifier based on whether current humidity is above or below target range

## Target Values

### Vegetative Stage
| Growth Phase | Target VPD Range |
|-------------|------------------|
| Early Veg   | 0.5-0.8 kPa     |
| Late Veg    | 0.8-1.0 kPa     |

### Flowering Stage
| Growth Phase   | Target VPD Range |
|---------------|------------------|
| Early Flower  | 0.8-1.0 kPa     |
| Mid Flower    | 1.0-1.2 kPa     |
| Late Flower   | 1.2-1.5 kPa     |

> **Note**: These values are for leaf VPD and will typically require higher humidity than equivalent air VPD targets.

## Required Hardware

- M5Stack CO2L Unit (SCD41) for air temperature and humidity
- M5Stack Thermal Camera for leaf temperature
- Smart plug with power monitoring
- Dehumidifier with power loss recovery

## Configuration

### Helpers

```yaml
input_number:
  target_vpd:
    name: Target Leaf VPD
    min: 0.5
    max: 2.0
    step: 0.1
    unit_of_measurement: kPa
    icon: mdi:water-percent
    
  humidity_tolerance:
    name: Humidity Tolerance
    min: 1
    max: 10
    step: 0.5
    unit_of_measurement: '%'
    icon: mdi:plus-minus
```

### Template Sensors

```yaml
template:
  - sensor:
      - name: "Humidity VPD Target Calculated"
        unit_of_measurement: "%"
        state: >
          {% set target_vpd = states('input_number.target_vpd') | float %}
          {% set t_air = states('sensor.scd_41_2_temperature') | float %}
          {% set t_leaf = states('sensor.espatom_mlx90640_mlx90640_mean_temp') | float %}
          {% set e = 2.71828 %}
          {% set asvp = 0.61078 * e ** (t_air / (t_air + 238.3) * 17.2694) %}
          {% set lsvp = 0.61078 * e ** (t_leaf / (t_leaf + 238.3) * 17.2694) %}
          {{ (100 * (lsvp - target_vpd) / asvp) | round(2) }}

      - name: "Leaf Temperature Difference"
        unit_of_measurement: "°C"
        state: >
          {% set ir_leaf_temp = states('sensor.espatom_mlx90640_mlx90640_max_temp') | float %}
          {% set middle_can_temp = states('sensor.scd_41_2_temperature') | float %}
          {{ (middle_can_temp - ir_leaf_temp) | round(2) }}
```

### Automation

```yaml
alias: Leaf VPD Humidity Control
description: "Controls dehumidifier based on VPD target humidity"
trigger:
  - platform: state
    entity_id: 
      - input_number.humidity_tolerance
      - sensor.scd_41_2_humidity
      - sensor.sensor_humidity_vpd_target_calculated
condition:
  - condition: numeric_state
    entity_id: sensor.scd_41_2_humidity
    below: "{{ states('sensor.sensor_humidity_vpd_target_calculated') | float + states('input_number.humidity_tolerance') | float }}"
    then:
      service: switch.turn_on
      target:
        entity_id: switch.t2
  - condition: numeric_state
    entity_id: sensor.scd_41_2_humidity
    above: "{{ states('sensor.sensor_humidity_vpd_target_calculated') | float - states('input_number.humidity_tolerance') | float }}"
    then:
      service: switch.turn_off
      target:
        entity_id: switch.t2
mode: single
```

### Entity Names to Update

```yaml
# Air Sensors (from SCD41)
sensor.scd_41_2_temperature          # Air temperature
sensor.scd_41_2_humidity             # Air humidity

# Leaf Temperature Sensors (from Thermal Camera)
sensor.espatom_mlx90640_mlx90640_mean_temp    # Average leaf temperature
sensor.espatom_mlx90640_mlx90640_max_temp     # Maximum leaf temperature

# Dehumidifier Control
switch.t2    # Smart plug controlling dehumidifier
```

## Mathematical Logic Explained

### VPD Calculation Process

1. **Saturated Vapor Pressure (SVP) Calculation**
   ```
   SVP = 0.61078 * e^((temperature)/(temperature + 238.3) * 17.2694)
   ```
   - Calculated for both leaf temperature (LSVP) and air temperature (ASVP)
   - Temperature must be in Celsius
   - Result is in kPa

2. **Target Humidity Calculation**
   ```
   Target RH = 100 * (LSVP - target_vpd) / ASVP
   ```
   Where:
   - LSVP = Leaf Saturated Vapor Pressure
   - ASVP = Air Saturated Vapor Pressure
   - target_vpd = Your desired Leaf VPD in kPa

### Control Band Logic
```
Upper Limit = Target RH + Tolerance
Lower Limit = Target RH - Tolerance
```
- Dehumidifier turns ON when humidity > Upper Limit
- Dehumidifier turns OFF when humidity < Lower Limit
- Creates a hysteresis band to prevent rapid cycling

## Common Issues and Solutions

### Temperature Differential Issues
| Issue | Possible Causes |
|-------|----------------|
| Leaf temp >2°C below air | • Excessive air movement<br>• Cold water pooling<br>• Root zone issues |

### Humidity Control Issues
| Problem | Solutions |
|---------|-----------|
| Wide humidity swings | • Increase tolerance value<br>• Check sensor placement<br>• Verify dehumidifier sizing |

### Target Adjustment Tips
- Increase target VPD gradually (0.1 kPa per day)
- Monitor leaf temperature differential
- Adjust based on plant response, not just numbers

## Best Practices

### 1. Sensor Placement
- Thermal camera: 30-60cm above canopy
- Air sensor: At canopy height
- Avoid direct light on sensors

### 2. Control Settings
- Start with 3% humidity tolerance
- Adjust based on dehumidifier cycle frequency
- Monitor leaf temperature differential

### 3. VPD Targets
- Start conservative (higher humidity)
- Increase gradually with plant development
- Adjust based on plant response
