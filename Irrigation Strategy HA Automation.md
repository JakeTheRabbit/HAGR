# Home Assistant Automation Script

## Description

"1.5 Combined Nutrient and Irrigation Automation" in Home Assistant orchestrates the irrigation and nutrient dosing process across different plant growth phases, adapting to the plants' needs through sensor feedback and scheduled intervals.

Phase Logic and Transition:

P0 (Ramp Up): This preparatory phase starts with the lights turning on. The system transitions from P0 to P1 when the rockwool’s humidity drops below the P3 dryback target, indicating it's time to begin initial irrigation.

P1 (Initial Irrigation): The pump delivers water and nutrients based on a predefined shot size. After dispensing a shot, the pump shuts off, and a timer defined by input_datetime.1_5_p1_shot_interval begins. The system remains in P1 until the rockwool’s humidity reaches the P1 field capacity, signaling a shift to P2.

P2 (Maintenance): The automation adjusts watering based on the rockwool's moisture levels to maintain the ideal VWC, using sensor data to trigger the pump as needed.

P3 (Dryback): The system encourages the medium to dry to a target level. The pump is deactivated to allow the moisture levels to fall to the P3 dryback target. The dryback phase ends based on a time schedule, moving back to P0 or P1 as needed.

Automation Triggers and Actions:

The triggers include time-based schedules for each phase's start time and sensor-based triggers that react to changes in the rockwool's calibrated humidity.

For P1, an additional trigger is the pump turning off, which starts a delay based on input_datetime.1_5_p1_shot_interval. Once the delay elapses, if the system is still in P1 and the conditions are met, the pump is reactivated for another shot.

The P2 and P3 phases are governed by the rockwool's humidity readings, either maintaining moisture levels or allowing the medium to dry.

Modification Summary:

The automation no longer relies on a hardcoded delay for the P1 phase but instead uses a dynamic interval managed by input_datetime.1_5_p1_shot_interval. This allows for more precise control based on the actual time elapsed after the pump turns off.

Additionally, the P1 phase uses a template condition to ensure that the pump doesn't turn back on until the interval specified by input_datetime.1_5_p1_shot_interval has passed since the last pump off time.

Dashboard helpers / control. This is very primitive but enough to get me going. 

![image](https://github.com/JakeTheRabbit/HAGR/assets/123831499/9fea4d92-4586-4c8d-b9c4-12e1a6ac3fd6)

Here is the template to copy into configuration.yaml to create the helpers. 

```
input_select:
  1_5_irrigation_phase:
    name: '1.5 Irrigation Phase'
    options:
      - 'P0'
      - 'P1'
      - 'P2'
      - 'P3'
    initial: 'P0'
    icon: mdi:water-pump

input_datetime:
  1_5_p1_start_time:
    name: 'P1 Start Time (Override)'
    has_date: false
    has_time: true
    initial: '05:00'

  1_5_last_pump_off_time:
    name: 'Last Pump Off Time'
    has_date: false
    has_time: true
    initial: '00:00'

  1_5_p3_start_time:
    name: 'P3 Start Time'
    has_date: false
    has_time: true
    initial: '21:00'

input_number:
  1_5_number_of_plants:
    name: 'Number of Plants'
    initial: 10
    min: 1
    max: 100
    step: 1

  1_5_field_capacity:
    name: '1.5 Field Capacity (triggers P2)'
    initial: 20
    min: 0
    max: 100
    step: 0.1

  1_5_p1_shot_size:
    name: 'P1 Shot Size'
    initial: 5
    min: 1
    max: 100
    step: 0.1

  1_5_p1_shot_volume:
    name: '1.5 P1 Shot Volume (Updates automatically)'
    initial: 1
    min: 0
    max: 1000
    step: 0.1

  1_5_p2_dryback_vwc:
    name: 'P2 Dryback VWC'
    initial: 10
    min: 0
    max: 100
    step: 0.1

  1_5_p2_field_capacity:
    name: 'P2 Field Capacity'
    initial: 15
    min: 0
    max: 100
    step: 0.1

  1_5_p3_dryback_target:
    name: 'P3 Dryback Target'
    initial: 5
    min: 0
    max: 100
    step: 0.1
```

You can copy this card to your dashboard with all of the entities you just configured. 

```

type: entities
entities:
  - entity: sensor.espatom_mtec_w2_rockwool_calibrated_humidity_2
    secondary_info: last-changed
    icon: mdi:water-percent
    name: Volumetric Water Content (VWC)
  - entity: sensor.espatom_mtec_w2_rockwool_pwec_2
    secondary_info: last-changed
    name: pwEC (Pore Water Electrical Conductivity)
    icon: mdi:omega
  - entity: switch.tp_link_m_m3
  - entity: input_select.1_5_irrigation_phase
  - entity: input_datetime.1_5_p1_start_time
    name: P1 Start Time (Override)
    secondary_info: last-updated
  - entity: input_number.1_5_number_of_plants
  - entity: input_number.1_5_field_capacity
    name: 1.5 Field Capacity (triggers P2)
  - entity: input_datetime.1_5_p1_shot_interval
  - entity: input_number.1_5_p1_shot_size
  - entity: input_number.1_5_p1_shot_volume
    name: 1.5 P1 Shot Volume (Updates automatically)
    icon: mdi:needle
  - entity: input_number.1_5_p2_dryback_vwc
  - entity: input_number.1_5_p2_field_capacity
  - entity: input_datetime.1_5_p3_start_time
  - entity: input_number.1_5_p3_dryback_target
  - entity: automation.turn_off_1_5_tent_water_after_3_minutes
  - entity: input_datetime.1_5_last_pump_off_time

```

Āutomation:
(This is beta as fuck aka don't trust it)

```
alias: 1.5 Combined Nutrient and Irrigation Automation
description: >
  This automation manages nutrient dosing and irrigation for P0 (ramp up from
  lights on to P3 dryback), P1 (initial irrigation phase), P2 (maintenance
  phase), and P3 (dryback phase).
trigger:
  - platform: time
    at: input_datetime.1_5_lights_on_time
  - platform: time
    at: input_datetime.1_5_p1_start_time
  - platform: time
    at: input_datetime.1_5_p3_start_time
  - platform: state
    entity_id: sensor.espatom_mtec_w2_rockwool_calibrated_humidity_2
  - platform: state
    entity_id: input_number.1_5_p1_shot_size
  - platform: state
    entity_id: input_select.1_5_irrigation_phase
  - platform: state
    entity_id: switch.tp_link_m_m3
    to: "off"
    for:
      hours: "{{ states('input_datetime.1_5_p1_shot_interval')[0:2] | int }}"
      minutes: "{{ states('input_datetime.1_5_p1_shot_interval')[3:5] | int }}"
      seconds: "{{ states('input_datetime.1_5_p1_shot_interval')[6:8] | int }}"
  - platform: time
    at: input_datetime.1_5_lights_on_time
  - platform: numeric_state
    entity_id: sensor.espatom_mtec_w2_rockwool_calibrated_humidity_2
    below: input_number.1_5_p3_dryback_target
condition: []
action:
  - choose:
      - conditions: # This condition checks if it's the lights on time to set phase to P0
          - condition: template
            value_template: "{{ now().time() == states('input_datetime.1_5_lights_on_time') }}"
        sequence:
          - service: input_select.select_option
            data:
              entity_id: input_select.1_5_irrigation_phase
              option: 'P0'
      - conditions: # This condition checks if the VWC is below target to set phase to P1
          - condition: numeric_state
            entity_id: sensor.espatom_mtec_w2_rockwool_calibrated_humidity_2
            below: input_number.1_5_p3_dryback_target
        sequence:
          - service: input_select.select_option
            data:
              entity_id: input_select.1_5_irrigation_phase
              option: 'P1'
  - choose:
      - conditions:
          - condition: state
            entity_id: input_select.1_5_irrigation_phase
            state: P0
        sequence: null
      - conditions:
          - condition: state
            entity_id: input_select.1_5_irrigation_phase
            state: P1
          - condition: template
            value_template: >
              {{ as_timestamp(now()) -
              as_timestamp(states('input_datetime.1_5_last_pump_off_time')) >
                 (states('input_datetime.1_5_p1_shot_interval').hour * 3600 +
                  states('input_datetime.1_5_p1_shot_interval').minute * 60 +
                  states('input_datetime.1_5_p1_shot_interval').second) }}
        sequence:
          - service: switch.turn_on
            entity_id: switch.tp_link_m_m3
          - delay:
              seconds: "{{ states('input_number.1_5_p1_shot_size') | float * 35 }}"
          - service: switch.turn_off
            entity_id: switch.tp_link_m_m3
          - delay:
              hours: "{{ states('input_datetime.1_5_p1_shot_interval')[0:2] | int }}"
              minutes: "{{ states('input_datetime.1_5_p1_shot_interval')[3:5] | int }}"
              seconds: "{{ states('input_datetime.1_5_p1_shot_interval')[6:8] | int }}"
          - service: input_datetime.set_datetime
            entity_id: input_datetime.1_5_last_pump_off_time
            data:
              timestamp: "{{ as_timestamp(now()) }}"
      - conditions:
          - condition: state
            entity_id: input_select.1_5_irrigation_phase
            state: P2
        sequence:
          - choose:
              - conditions:
                  - condition: numeric_state
                    entity_id: sensor.espatom_mtec_w2_rockwool_calibrated_humidity_2
                    below: input_number.1_5_p2_dryback_vwc
                sequence:
                  - service: switch.turn_on
                    entity_id: switch.tp_link_m_m3
              - conditions:
                  - condition: numeric_state
                    entity_id: sensor.espatom_mtec_w2_rockwool_calibrated_humidity_2
                    above: input_number.1_5_p2_field_capacity
                sequence:
                  - service: switch.turn_off
                    entity_id: switch.tp_link_m_m3
      - conditions:
          - condition: state
            entity_id: input_select.1_5_irrigation_phase
            state: P3
        sequence:
          - choose:
              - conditions:
                  - condition: numeric_state
                    entity_id: sensor.espatom_mtec_w2_rockwool_calibrated_humidity_2
                    below: input_number.1_5_p3_dryback_target
                sequence:
                  - service: switch.turn_off
                    entity_id: switch.tp_link_m_m3
                  - delay: "00:00:01"
                  - service: switch.turn_on
                    entity_id: switch.tp_link_m_m3
                  - delay:
                      seconds: >-
                        {{ states('input_number.1_5_p1_shot_size') | float * 35
                        }}
                  - service: switch.turn_off
                    entity_id: switch.tp_link_m_m3
mode: single

```

## Contributing 
Feel free to fork this repository and submit pull requests with your own improvements to the automation script.

For any issues or suggestions, please open an issue in the repository.



