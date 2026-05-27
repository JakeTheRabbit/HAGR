# Derived Crop Signals from Leaf, Climate, Light, CO₂, and Root-Zone Sensors

_Practical inference layer for controlled-environment cannabis cultivation using leaf temperature, ambient temperature, RH, PPFD, CO₂, root-zone temperature, root-zone EC, and root-zone VWC._

> [!NOTE]
> **Core objective:** use combined sensor relationships to determine whether the crop is maintaining transpiration and photosynthetic capacity under the current light, CO₂, climate, and root-zone conditions.

> [!WARNING]
> **Potency/yield limitation:** these derived signals do not directly measure cannabinoid or terpene potency. They help maintain the plant in a productive operating window by improving control over transpiration, root-zone supply, atmospheric demand, and stress timing. Final potency still depends heavily on genetics, crop stage, harvest timing, nutrition, plant health, and post-harvest handling.

## Contents

- [Knowns, assumptions, and unknowns](#knowns-assumptions-and-unknowns)
- [Importance-ranked derived signals](#importance-ranked-derived-signals)
- [Core maths and formulas](#core-maths-and-formulas)
- [Home Assistant template examples](#home-assistant-template-examples)
- [Decision table](#decision-table)
- [Recommended implementation order](#recommended-implementation-order)

## Knowns, assumptions, and unknowns

| Category | Details |
|---|---|
| **Knowns** | Available sensors: leaf surface temperature, ambient temperature, relative humidity, root-zone temperature, root-zone EC, root-zone VWC, PPFD, and CO₂. |
| **Assumptions** | Sensors are fixed in stable positions, logged over time, reasonably calibrated, and viewed as trend instruments rather than absolute truth. The IR leaf sensor is assumed to have consistent geometry and field-of-view. |
| **Unknowns** | Cultivar, crop stage, media type, irrigation strategy, EC target, dryback target, CO₂ setpoint, PPFD limit, VPD target, and whether the IR leaf reading is representative of the canopy. |

## Importance-ranked derived signals

| Rank | Derived signal | Inputs | How it works | Why it is useful | How it can support potency and yield | Main caveats |
|---|---|---|---|---|---|---|
| 1 | **Leaf–air temperature delta** | Leaf temp, ambient temp | Calculates `leaf_temp - ambient_temp`. Negative means the leaf is cooler than air. Positive means the leaf is warmer than air. | This is one of the clearest plant-response signals. It shows whether the canopy is maintaining evaporative cooling. | Helps determine whether PPFD, CO₂, and VPD are being pushed within the plant’s capacity. A crop that stays cooler than air under strong light is usually transpiring more effectively, supporting carbon assimilation and biomass production. | Single-point IR readings may not represent the whole canopy. Field-of-view contamination can mislead. |
| 2 | **Air VPD** | Ambient temp, RH | Calculates the evaporative demand of the air from temperature and relative humidity. | Shows how hard the room is pulling water from the plant. | Proper VPD supports stomatal function, transpiration, nutrient movement, and growth rate. Stable VPD reduces uncontrolled stress and improves consistency. | Air VPD does not prove the plant is keeping up. Pair it with leaf temperature. |
| 3 | **Leaf VPD** | Leaf temp, ambient temp, RH | Uses actual vapour pressure from ambient air, but saturation vapour pressure at leaf temperature. | More plant-relevant than air VPD alone when leaf temperature differs from air temperature. | Helps prevent excessive leaf-level evaporative demand during high PPFD or high CO₂ operation. This protects assimilation and reduces unnecessary stress. | Do not use leaf VPD alone as a stress signal. Warm leaves can have multiple causes. |
| 4 | **Transpiration cooling index** | Ambient temp, leaf temp | Calculates `ambient_temp - leaf_temp`. Higher positive values indicate more cooling. | Simple dashboard metric for how strongly the canopy is cooling itself. | Useful for validating irrigation timing, VPD settings, light intensity, and airflow. Better transpiration control supports yield consistency and reduces avoidable stress. | High cooling under low VPD may reflect weak demand, not superior performance. |
| 5 | **VWC dryback rate** | Root-zone VWC over time | Tracks how quickly substrate water content falls after irrigation. | Shows crop water use pattern and irrigation timing quality. | Helps avoid excessive dryback or persistently wet media. Controlled dryback supports oxygen availability, root function, nutrient uptake, and crop steering. | Highly dependent on probe placement and media uniformity. |
| 6 | **EC concentration trend** | Root-zone EC, VWC over time | Tracks EC movement relative to dryback and irrigation events. | Shows whether salts are accumulating, diluting, or behaving unexpectedly. | Helps keep root-zone salinity within a productive range. Avoiding osmotic stress protects growth while still allowing intentional generative steering where appropriate. | Root-zone EC is not the same as feed EC or runoff EC. Media and probe type matter. |
| 7 | **Substrate stress flag** | VWC, EC, root-zone temp, leaf–air delta | Flags combinations such as low VWC, high EC, poor root temperature, and warming leaves. | Separates productive dryback from water/osmotic/root-zone stress. | Prevents hidden yield loss from water limitation, high salinity, or poor root function. Helps maintain enough root-zone supply to support high-light operation. | Thresholds must be crop-, media-, and stage-specific. |
| 8 | **Irrigation response test** | VWC, EC, leaf temp, ambient temp, VPD | Compares before and after irrigation. A good response is VWC rising, EC diluting or stabilising, leaf temp falling, and leaf–air delta becoming more negative. | Shows whether irrigation actually improved plant hydraulic status. | Optimises shot size, frequency, and timing. If leaf cooling improves after irrigation, the prior condition was likely limiting transpiration and growth. | If leaf temp does not improve, the issue may be airflow, root health, EC, PPFD, or sensor geometry. |
| 9 | **Light-load stress flag** | PPFD, leaf temp, air VPD, CO₂ | Flags high PPFD combined with warming leaves and demanding VPD. | Identifies when light load exceeds the plant’s cooling or assimilation capacity. | Helps increase PPFD only when the plant can use it. This improves grams per kWh and reduces photoinhibition, heat stress, or stomatal closure risk. | High PPFD is only useful when CO₂, VPD, root-zone conditions, and nutrition support it. |
| 10 | **CO₂ usefulness flag** | CO₂, PPFD, leaf–air delta, VPD | Checks whether CO₂ is elevated during high PPFD and whether the plant is still maintaining leaf cooling. | Separates “CO₂ is present” from “CO₂ is likely useful.” | Prevents wasting CO₂ when light, water, VPD, or stomatal function is limiting. Supports better alignment between light intensity and enrichment strategy. | Photosynthesis is not being measured directly. This is a proxy. |
| 11 | **CWSI-style water stress index** | Leaf–air delta, air VPD | Compares measured leaf–air delta against a non-water-stressed baseline and a water-stressed baseline. | Normalises leaf temperature response against atmospheric demand. | Helps detect reduced transpiration capacity under current demand. Useful for avoiding silent stress during aggressive dryback, lighting, or CO₂ strategies. | Needs your own cannabis-specific baseline. Generic field-crop baselines are not reliable. |
| 12 | **Root-zone temperature modifier** | Root-zone temp, VWC, EC, leaf temp | Interprets water and EC readings in the context of root temperature. | The same VWC/EC can behave differently when roots are cold, warm, or hypoxic. | Maintaining productive root-zone temperature supports uptake, oxygen balance, root metabolism, and transpiration capacity. | Target range depends on cultivar, biology, media, and irrigation strategy. |
| 13 | **Night recovery / overnight dryback** | VWC, EC, root temp, ambient temp/RH, leaf temp if useful | Looks at substrate behaviour and recovery during lights-off. | Detects overwatering, stagnation, drainage issues, leaks, or unexpected night demand. | Better night recovery reduces chronic root stress and disease pressure, supporting stronger daytime transpiration and yield. | Night leaf IR readings may be less useful depending on sensor placement and environment. |
| 14 | **PPFD-to-leaf-temp response curve** | PPFD, leaf temp, ambient temp, VPD | Tracks how leaf temperature responds as PPFD changes. | Finds the point where extra light starts creating disproportionate canopy heat or stress. | Helps tune usable light rather than simply maximising photons. This supports yield efficiency and reduces overdriving risk. | Requires stable airflow and climate to compare properly. |
| 15 | **EC/VWC irrigation uniformity check** | VWC, EC before/after irrigation | Checks whether each irrigation event produces expected VWC rise and EC dilution/stabilisation. | Detects channeling, poor distribution, insufficient shot size, or probe placement issues. | More uniform wetting reduces localised salinity and supports more even canopy development. | One probe cannot prove whole-container uniformity. |

## Core maths and formulas

### 1. Saturation vapour pressure

Use temperature in °C. Result is in kPa.

```text
SVP(T) = 0.6108 × exp((17.27 × T) / (T + 237.3))
```

### 2. Actual vapour pressure

Use ambient air temperature and RH. Do not use leaf temperature to calculate actual vapour pressure.

```text
AVP = SVP(air_temp_C) × RH_percent / 100
```

### 3. Air VPD

```text
air_vpd_kpa = SVP(air_temp_C) - AVP
```

### 4. Leaf VPD

Leaf VPD uses leaf temperature for saturation vapour pressure, but still uses air-derived actual vapour pressure.

```text
leaf_vpd_kpa = SVP(leaf_temp_C) - AVP
```

### 5. Leaf–air temperature delta

```text
leaf_air_delta_C = leaf_temp_C - air_temp_C
```

| Result | Likely interpretation |
|---|---|
| `leaf_air_delta_C < 0` | Leaf cooler than air. Usually indicates active transpiration / evaporative cooling. |
| `leaf_air_delta_C ≈ 0` | Leaf near air temperature. Moderate cooling or weak demand. |
| `leaf_air_delta_C > 0` | Leaf warmer than air. Possible stomatal restriction, water limitation, high radiation load, poor airflow, or sensor issue. |

### 6. Transpiration cooling index

```text
transpiration_cooling_C = air_temp_C - leaf_temp_C
```

This is the inverse of leaf–air delta. A larger positive number means stronger leaf cooling.

### 7. Leaf VPD minus air VPD

```text
leaf_vpd_minus_air_vpd = leaf_vpd_kpa - air_vpd_kpa
```

| Result | Interpretation |
|---|---|
| Negative | Leaf is cooler than air. Actual leaf-level demand is lower than air VPD implies. |
| Near zero | Leaf is close to air temperature. |
| Positive | Leaf is warmer than air. Actual leaf-level demand is higher than air VPD implies. |

### 8. VWC dryback rate

Simple rate of change over a time interval.

```text
dryback_rate_percent_per_hour = (VWC_now - VWC_previous) / hours_elapsed
```

During normal dryback this value is usually negative. A more negative value means faster dryback.

### 9. EC concentration trend

```text
ec_trend_per_hour = (EC_now - EC_previous) / hours_elapsed
```

EC often rises during dryback as water is removed and salts concentrate. EC should usually dilute or stabilise after irrigation.

### 10. CWSI-style index

A practical CWSI-style signal compares actual leaf–air delta against an expected non-water-stressed baseline. It should be calibrated from your own healthy crop data.

```text
NWSB = nwsb_slope × air_vpd_kpa + nwsb_intercept
```

```text
WSB = configured_water_stressed_delta_C
```

```text
CWSI_raw = (leaf_air_delta_C - NWSB) / (WSB - NWSB)
```

```text
CWSI_clamped = min(1, max(0, CWSI_raw))
```

| CWSI-style value | Interpretation |
|---|---|
| Near 0 | Crop behaving close to the well-watered baseline. |
| Rising | Reduced transpiration relative to atmospheric demand. |
| Near 1 | High stress signal. Could be water stress, osmotic stress, root issue, airflow issue, or excessive light load. |
| Below 0 | Leaf cooler than expected or baseline needs tuning. |
| Above 1 | More stressed than configured water-stressed baseline or parameters are wrong. |

## Home Assistant template examples

> [!WARNING]
> **Important:** replace the entity IDs below with your actual sensor entity IDs. Treat thresholds as placeholders until calibrated against your own crop, media, cultivar, and stage.

### Template sensors

```yaml
template:
  - sensor:
      - name: "Canopy Air VPD"
        unique_id: canopy_air_vpd
        unit_of_measurement: "kPa"
        state_class: measurement
        state: >
          {% set t = states('sensor.ambient_temperature') | float(none) %}
          {% set rh = states('sensor.relative_humidity') | float(none) %}
          {% if t is none or rh is none %}
            unknown
          {% else %}
            {% set svp = 0.6108 * e ** ((17.27 * t) / (t + 237.3)) %}
            {% set avp = svp * rh / 100 %}
            {{ (svp - avp) | round(3) }}
          {% endif %}

      - name: "Canopy Leaf VPD"
        unique_id: canopy_leaf_vpd
        unit_of_measurement: "kPa"
        state_class: measurement
        state: >
          {% set leaf_t = states('sensor.leaf_surface_temperature') | float(none) %}
          {% set air_t = states('sensor.ambient_temperature') | float(none) %}
          {% set rh = states('sensor.relative_humidity') | float(none) %}
          {% if leaf_t is none or air_t is none or rh is none %}
            unknown
          {% else %}
            {% set air_svp = 0.6108 * e ** ((17.27 * air_t) / (air_t + 237.3)) %}
            {% set avp = air_svp * rh / 100 %}
            {% set leaf_svp = 0.6108 * e ** ((17.27 * leaf_t) / (leaf_t + 237.3)) %}
            {{ (leaf_svp - avp) | round(3) }}
          {% endif %}

      - name: "Leaf Air Delta"
        unique_id: leaf_air_delta
        unit_of_measurement: "°C"
        state_class: measurement
        state: >
          {% set leaf_t = states('sensor.leaf_surface_temperature') | float(none) %}
          {% set air_t = states('sensor.ambient_temperature') | float(none) %}
          {% if leaf_t is none or air_t is none %}
            unknown
          {% else %}
            {{ (leaf_t - air_t) | round(2) }}
          {% endif %}

      - name: "Transpiration Cooling Index"
        unique_id: transpiration_cooling_index
        unit_of_measurement: "°C"
        state_class: measurement
        state: >
          {% set leaf_t = states('sensor.leaf_surface_temperature') | float(none) %}
          {% set air_t = states('sensor.ambient_temperature') | float(none) %}
          {% if leaf_t is none or air_t is none %}
            unknown
          {% else %}
            {{ (air_t - leaf_t) | round(2) }}
          {% endif %}

      - name: "Leaf VPD Minus Air VPD"
        unique_id: leaf_vpd_minus_air_vpd
        unit_of_measurement: "kPa"
        state_class: measurement
        state: >
          {% set leaf_vpd = states('sensor.canopy_leaf_vpd') | float(none) %}
          {% set air_vpd = states('sensor.canopy_air_vpd') | float(none) %}
          {% if leaf_vpd is none or air_vpd is none %}
            unknown
          {% else %}
            {{ (leaf_vpd - air_vpd) | round(3) }}
          {% endif %}

      - name: "Root Zone VWC Dryback Rate"
        unique_id: root_zone_vwc_dryback_rate
        unit_of_measurement: "%/h"
        state_class: measurement
        state: >
          {% set current = states('sensor.root_zone_vwc') | float(none) %}
          {% set previous = states('input_number.previous_root_zone_vwc') | float(none) %}
          {% set hours = states('input_number.vwc_rate_hours_elapsed') | float(1) %}
          {% if current is none or previous is none or hours <= 0 %}
            unknown
          {% else %}
            {{ ((current - previous) / hours) | round(2) }}
          {% endif %}

      - name: "Root Zone EC Trend"
        unique_id: root_zone_ec_trend
        unit_of_measurement: "EC/h"
        state_class: measurement
        state: >
          {% set current = states('sensor.root_zone_ec') | float(none) %}
          {% set previous = states('input_number.previous_root_zone_ec') | float(none) %}
          {% set hours = states('input_number.ec_rate_hours_elapsed') | float(1) %}
          {% if current is none or previous is none or hours <= 0 %}
            unknown
          {% else %}
            {{ ((current - previous) / hours) | round(2) }}
          {% endif %}

      - name: "Canopy CWSI Raw"
        unique_id: canopy_cwsi_raw
        state_class: measurement
        state: >
          {% set delta = states('sensor.leaf_air_delta') | float(none) %}
          {% set air_vpd = states('sensor.canopy_air_vpd') | float(none) %}
          {% set nwsb_slope = states('input_number.cwsi_nwsb_slope') | float(-2.0) %}
          {% set nwsb_intercept = states('input_number.cwsi_nwsb_intercept') | float(1.5) %}
          {% set wsb = states('input_number.cwsi_wsb_delta') | float(3.5) %}

          {% if delta is none or air_vpd is none %}
            unknown
          {% else %}
            {% set nwsb = nwsb_slope * air_vpd + nwsb_intercept %}
            {% set denom = wsb - nwsb %}
            {% if denom | abs < 0.01 %}
              unknown
            {% else %}
              {{ ((delta - nwsb) / denom) | round(3) }}
            {% endif %}
          {% endif %}

      - name: "Canopy CWSI Clamped"
        unique_id: canopy_cwsi_clamped
        state_class: measurement
        state: >
          {% set raw = states('sensor.canopy_cwsi_raw') | float(none) %}
          {% if raw is none %}
            unknown
          {% else %}
            {{ ([0, [raw, 1] | min] | max) | round(3) }}
          {% endif %}
```

### Helper inputs

```yaml
input_number:
  cwsi_nwsb_slope:
    name: CWSI NWSB Slope
    min: -5
    max: 1
    step: 0.01
    initial: -2.0

  cwsi_nwsb_intercept:
    name: CWSI NWSB Intercept
    min: -5
    max: 5
    step: 0.01
    initial: 1.5

  cwsi_wsb_delta:
    name: CWSI Water Stressed Baseline Delta
    min: 0
    max: 8
    step: 0.1
    initial: 3.5
    unit_of_measurement: "°C"

  previous_root_zone_vwc:
    name: Previous Root Zone VWC
    min: 0
    max: 100
    step: 0.1
    unit_of_measurement: "%"

  vwc_rate_hours_elapsed:
    name: VWC Rate Hours Elapsed
    min: 0.1
    max: 24
    step: 0.1
    initial: 1

  previous_root_zone_ec:
    name: Previous Root Zone EC
    min: 0
    max: 20
    step: 0.1

  ec_rate_hours_elapsed:
    name: EC Rate Hours Elapsed
    min: 0.1
    max: 24
    step: 0.1
    initial: 1
```

### Example binary stress flags

```yaml
template:
  - binary_sensor:
      - name: "Substrate Stress Flag"
        unique_id: substrate_stress_flag
        state: >
          {% set vwc = states('sensor.root_zone_vwc') | float(none) %}
          {% set ec = states('sensor.root_zone_ec') | float(none) %}
          {% set root_t = states('sensor.root_zone_temperature') | float(none) %}
          {% set delta = states('sensor.leaf_air_delta') | float(none) %}

          {% if vwc is none or ec is none or root_t is none or delta is none %}
            false
          {% else %}
            {{
              (
                vwc < states('input_number.low_vwc_threshold') | float(35)
                and ec > states('input_number.high_ec_threshold') | float(6)
                and delta > states('input_number.warm_leaf_delta_threshold') | float(0)
              )
              or root_t < states('input_number.low_root_temp_threshold') | float(18)
              or root_t > states('input_number.high_root_temp_threshold') | float(26)
            }}
          {% endif %}

      - name: "Light Load Stress Flag"
        unique_id: light_load_stress_flag
        state: >
          {% set ppfd = states('sensor.ppfd') | float(none) %}
          {% set delta = states('sensor.leaf_air_delta') | float(none) %}
          {% set air_vpd = states('sensor.canopy_air_vpd') | float(none) %}

          {% if ppfd is none or delta is none or air_vpd is none %}
            false
          {% else %}
            {{
              ppfd > states('input_number.high_ppfd_threshold') | float(900)
              and delta > states('input_number.warm_leaf_delta_threshold') | float(0)
              and air_vpd > states('input_number.high_vpd_threshold') | float(1.4)
            }}
          {% endif %}

      - name: "CO2 Likely Useful Flag"
        unique_id: co2_likely_useful_flag
        state: >
          {% set co2 = states('sensor.co2') | float(none) %}
          {% set ppfd = states('sensor.ppfd') | float(none) %}
          {% set cooling = states('sensor.transpiration_cooling_index') | float(none) %}

          {% if co2 is none or ppfd is none or cooling is none %}
            false
          {% else %}
            {{
              co2 > states('input_number.co2_enriched_threshold') | float(800)
              and ppfd > states('input_number.co2_useful_ppfd_threshold') | float(700)
              and cooling > states('input_number.minimum_leaf_cooling_threshold') | float(0.5)
            }}
          {% endif %}
```

### Threshold helper examples

```yaml
input_number:
  low_vwc_threshold:
    name: Low VWC Threshold
    min: 0
    max: 100
    step: 0.1
    initial: 35
    unit_of_measurement: "%"

  high_ec_threshold:
    name: High Root Zone EC Threshold
    min: 0
    max: 20
    step: 0.1
    initial: 6

  warm_leaf_delta_threshold:
    name: Warm Leaf Delta Threshold
    min: -5
    max: 5
    step: 0.1
    initial: 0
    unit_of_measurement: "°C"

  low_root_temp_threshold:
    name: Low Root Temperature Threshold
    min: 5
    max: 30
    step: 0.1
    initial: 18
    unit_of_measurement: "°C"

  high_root_temp_threshold:
    name: High Root Temperature Threshold
    min: 15
    max: 40
    step: 0.1
    initial: 26
    unit_of_measurement: "°C"

  high_ppfd_threshold:
    name: High PPFD Threshold
    min: 0
    max: 2000
    step: 10
    initial: 900
    unit_of_measurement: "µmol/m²/s"

  high_vpd_threshold:
    name: High VPD Threshold
    min: 0
    max: 3
    step: 0.01
    initial: 1.4
    unit_of_measurement: "kPa"

  co2_enriched_threshold:
    name: CO2 Enriched Threshold
    min: 400
    max: 2000
    step: 10
    initial: 800
    unit_of_measurement: "ppm"

  co2_useful_ppfd_threshold:
    name: CO2 Useful PPFD Threshold
    min: 0
    max: 2000
    step: 10
    initial: 700
    unit_of_measurement: "µmol/m²/s"

  minimum_leaf_cooling_threshold:
    name: Minimum Leaf Cooling Threshold
    min: -5
    max: 5
    step: 0.1
    initial: 0.5
    unit_of_measurement: "°C"
```

## Decision table

| Observed pattern | Likely interpretation | Recommended response |
|---|---|---|
| High PPFD + good CO₂ + leaf cooler than air + stable VWC/EC | Crop is probably handling the current load. | Maintain strategy or cautiously increase DLI if other crop indicators support it. |
| High PPFD + leaf warming + VWC falling + EC rising | Water limitation and/or osmotic stress likely. | Irrigate sooner, reduce dryback severity, review shot size and root-zone EC. |
| High PPFD + leaf warming + VWC adequate + EC high | Osmotic restriction likely. | Review feed EC, runoff strategy, salt accumulation, and irrigation frequency. |
| High PPFD + leaf warming + VWC adequate + EC normal | Airflow, root health, excessive PPFD, root temperature, or sensor geometry issue likely. | Check airflow, root-zone temperature, IR sensor aim, and light intensity. |
| High CO₂ + low PPFD | CO₂ likely underutilised. | Increase usable light only if canopy cooling and root-zone supply support it. |
| High CO₂ + high PPFD + warm leaves | CO₂ is probably not the limiting factor. | Fix VPD, irrigation, airflow, EC, root temperature, or PPFD before increasing CO₂. |
| VWC remains high overnight + EC unstable + weak daytime cooling | Possible overwatering, low oxygen, cold roots, or poor root function. | Review irrigation timing, night dryback, root-zone temperature, drainage, and biological/root health indicators. |
| Leaf temp improves after irrigation | Previous condition was likely limiting transpiration. | Use this response to tune irrigation timing and dryback limits. |

## Recommended implementation order

| Step | Build first | Reason |
|---|---|---|
| 1 | Leaf–air delta and air VPD | Immediate read on atmospheric demand versus actual plant cooling. |
| 2 | Leaf VPD and transpiration cooling index | Separates room demand from actual leaf-surface conditions. |
| 3 | VWC dryback rate and EC trend | Shows root-zone water supply and salinity movement. |
| 4 | Irrigation response test | Validates whether irrigation actually improves plant status. |
| 5 | Light-load stress flag | Prevents overdriving PPFD beyond plant capacity. |
| 6 | CO₂ usefulness flag | Avoids wasting CO₂ when light, VPD, or root-zone conditions are limiting. |
| 7 | CWSI-style index | Adds a normalised stress metric once baseline data exists. |

> [!TIP]
> **Primary operating question:** At the current PPFD, CO₂, and VPD, is the plant maintaining leaf cooling, and is the root zone supplying enough water without excessive EC stress?
