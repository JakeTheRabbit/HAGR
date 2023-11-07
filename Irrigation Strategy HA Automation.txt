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



alias: 1.5 Combined Nutrient and Irrigation Automation
description: >
  This automation manages nutrient dosing and irrigation for P0 (ramp up from
  lights on to P3 dryback), P1 (initial irrigation phase), P2 (maintenance
  phase), and P3 (dryback phase).
trigger:
  # Triggers for the automation based on time or sensor state changes
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
  # Additional trigger for P1 phase based on the pump's off state and the interval helper
  - platform: state
    entity_id: switch.tp_link_m_m3
    to: 'off'
    for: 
      hours: "{{ states('input_datetime.1_5_p1_shot_interval')[0:2] | int }}"
      minutes: "{{ states('input_datetime.1_5_p1_shot_interval')[3:5] | int }}"
      seconds: "{{ states('input_datetime.1_5_p1_shot_interval')[6:8] | int }}"
condition: []
action:
  - choose:
      # Actions for P0 Phase
      - conditions:
          - condition: state
            entity_id: input_select.1_5_irrigation_phase
            state: 'P0'
        sequence:
          # Sequence of actions for P0 phase

      # Actions for P1 Phase
      - conditions:
          - condition: state
            entity_id: input_select.1_5_irrigation_phase
            state: 'P1'
          # Additional condition to ensure the interval has passed after the pump turns off
          - condition: template
            value_template: >
              {{ as_timestamp(now()) - as_timestamp(states('input_datetime.1_5_last_pump_off_time')) >
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
          # Now, the delay is set by the input_datetime helper
          - delay:
              hours: "{{ states('input_datetime.1_5_p1_shot_interval')[0:2] | int }}"
              minutes: "{{ states('input_datetime.1_5_p1_shot_interval')[3:5] | int }}"
              seconds: "{{ states('input_datetime.1_5_p1_shot_interval')[6:8] | int }}"
          # Update the last pump off time after the delay
          - service: input_datetime.set_datetime
            entity_id: input_datetime.1_5_last_pump_off_time
            data:
              timestamp: "{{ as_timestamp(now()) }}"

      # Actions for P2 Phase
      - conditions:
          - condition: state
            entity_id: input_select.1_5_irrigation_phase
            state: 'P2'
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

      # Actions for P3 Phase
      - conditions:
          - condition: state
            entity_id: input_select.1_5_irrigation_phase
            state: 'P3'
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


