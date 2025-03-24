
<img width="1592" alt="tank_diagram" src="https://github.com/user-attachments/assets/e5f4646e-7a7a-4e2c-98f2-d07b2f0ff158" />


```
alias: Tank Low Level Refill and Dose
description: >-
  Manages tank refill and dosing process when water level drops below 0.6 or
  manual button press. Checks pH and EC values after dosing and alerts if out of
  range.
triggers:
  - entity_id:
      - sensor.aqua_pro_water_level
    below: 5
    for:
      minutes: 1
    id: water_level
    trigger: numeric_state
    enabled: false
  - entity_id: input_button.manual_tank_fill
    id: manual_button
    enabled: true
    trigger: state
  - trigger: state
    entity_id:
      - binary_sensor.tank_sensor_tank_level
    from: "on"
    to: "off"
  - trigger: numeric_state
    entity_id:
      - sensor.aqua_pro_water_level
    below: 0.06
conditions:
  - condition: state
    entity_id: input_boolean.tank_fill_safety_switch
    state: "on"
  - condition: numeric_state
    entity_id: sensor.aqua_pro_water_level
    below: 0.06
actions:
  - data:
      message: Starting tank fill sequence - Turning off irrigation
      data:
        tag: tank_fill_process
    action: notify.mobile_app_s23ultra
  - action: input_boolean.turn_on
    metadata: {}
    data: {}
    target:
      entity_id: input_boolean.nutrient_dosing_active
  - target:
      entity_id:
        - switch.f1_irrigation_relays_relay_1
        - switch.f1_irrigation_relays_relay_2
        - switch.f1_irrigation_relays_relay_3
        - switch.f1_irrigation_relays_relay_4
        - switch.waste_valve
    action: switch.turn_off
    data: {}
  - delay:
      hours: 0
      minutes: 0
      seconds: 5
  - action: switch.turn_off
    metadata: {}
    data: {}
    target:
      entity_id:
        - switch.espoe_irrigation_relay_1_2
        - switch.espoe_irrigation_relay_2_4
  - delay:
      hours: 0
      minutes: 0
      seconds: 5
  - data:
      message: Turning on tank fill valve
      data:
        tag: tank_fill_process
    action: notify.mobile_app_s23ultra
  - target:
      entity_id: switch.espoe_irrigation_relay_1_4
    action: switch.turn_on
    data: {}
  - data:
      message: Starting tank fill - Running for 10 minutes
      data:
        tag: tank_fill_process
    action: notify.mobile_app_s23ultra
  - delay:
      hours: 0
      minutes: 10
      seconds: 0
  - data:
      message: Tank fill complete - Starting circulation
      title: Tank Fill Progress
      data:
        tag: tank_fill_process
    action: notify.mobile_app_s23ultra
  - action: switch.turn_off
    metadata: {}
    data: {}
    target:
      entity_id: switch.pump_power_switch
  - action: switch.turn_off
    metadata: {}
    data: {}
    target:
      entity_id: switch.f1_irrigation_pump_master_switch
  - target:
      entity_id:
        - switch.espoe_irrigation_relay_2_4
        - switch.espoe_irrigation_relay_1_1
    action: switch.turn_on
    data: {}
  - delay:
      hours: 0
      minutes: 0
      seconds: 3
  - action: switch.turn_on
    metadata: {}
    data: {}
    target:
      entity_id: switch.pump_power_switch
  - delay:
      hours: 0
      minutes: 0
      seconds: 2
  - action: switch.turn_on
    metadata: {}
    data: {}
    target:
      entity_id: switch.f1_irrigation_pump_master_switch
  - delay:
      hours: 0
      minutes: 0
      seconds: 15
  - condition: numeric_state
    entity_id: sensor.pump_power_switch_power
    above: 100
  - data:
      message: Starting nutrient dosing
      title: Tank Fill Progress
      data:
        tag: tank_fill_process
    action: notify.mobile_app_s23ultra
  - condition: state
    entity_id: switch.espoe_irrigation_relay_1_1
    state: "on"
    enabled: false
  - choose:
      - conditions:
          - condition: numeric_state
            entity_id: sensor.pump_power_switch_power
            below: 100
        sequence:
          - target:
              entity_id: switch.f1_irrigation_pump_master_switch
            action: switch.turn_off
            data: {}
          - delay:
              seconds: 5
          - target:
              entity_id: switch.f1_irrigation_pump_master_switch
            action: switch.turn_on
            data: {}
          - delay:
              seconds: 10
          - data:
              message: "URGENT: Pump power below 100W after reset. Stopping automation."
              priority: high
              data:
                tag: tank_fill_error
            action: notify.mobile_app_s23ultra
          - stop: Pump power check failed
  - delay:
      hours: 0
      minutes: 0
      seconds: 30
  - data:
      topic: AQU1AD04A42/ctr
      payload: >-
        { "cmd": "setNutrient", "msgid": "start_dose", "sn": "AQU1AD04A42",
        "monitor": 0 }
    action: mqtt.publish
  - delay:
      hours: 0
      minutes: 3
      seconds: 0
      milliseconds: 0
  - data:
      message: Dosing complete - Starting mixing period
      data:
        tag: tank_fill_process
    action: notify.mobile_app_s23ultra
  - data:
      topic: AQU1AD04A42/ctr
      payload: >-
        { "cmd": "setNutrient", "msgid": "force_stop", "sn": "AQU1AD04A42",
        "monitor": 1 }
    action: mqtt.publish
  - data:
      message: Mixing complete - Shutting down pumps
      data:
        tag: tank_fill_process
    action: notify.mobile_app_s23ultra
  - target:
      entity_id:
        - switch.espoe_irrigation_relay_1_4
    action: switch.turn_off
    data: {}
  - delay:
      minutes: 1
  - target:
      entity_id:
        - switch.espoe_irrigation_relay_1_1
    action: switch.turn_off
    data: {}
  - delay:
      hours: 0
      minutes: 0
      seconds: 1
      milliseconds: 0
  - target:
      entity_id:
        - switch.espoe_irrigation_relay_2_4
    action: switch.turn_off
    data: {}
  - delay:
      minutes: 5
  - data:
      message: Checking pH and EC levels...
      data:
        tag: tank_fill_process
    action: notify.mobile_app_s23ultra
  - choose:
      - conditions:
          - condition: or
            conditions:
              - condition: numeric_state
                entity_id: sensor.atlas_legacy_1_ph_res1
                below: 5.8
              - condition: numeric_state
                entity_id: sensor.atlas_legacy_1_ph_res1
                above: 6.2
        sequence:
          - data:
              message: >-
                CRITICAL ALERT: pH out of range! Current pH: {{
                states('sensor.atlas_legacy_1_ph_res1') }}. Should be between
                5.80-6.20.
              title: pH Value Out of Range
              data:
                ttl: 0
                priority: high
                channel: critical_alerts
                importance: high
                vibrationPattern: 100, 1000, 100, 1000, 100
                tag: ph_ec_alert
            action: notify.mobile_app_s23ultra
      - conditions:
          - condition: or
            conditions:
              - condition: numeric_state
                entity_id: sensor.atlas_legacy_1_ec_res1
                below: 2.8
              - condition: numeric_state
                entity_id: sensor.atlas_legacy_1_ec_res1
                above: 3.2
        sequence:
          - data:
              message: >-
                CRITICAL ALERT: EC out of range! Current EC: {{
                states('sensor.atlas_legacy_1_ec_res1') }}. Should be between
                2.80-3.20.
              title: EC Value Out of Range
              data:
                ttl: 0
                priority: high
                channel: critical_alerts
                importance: high
                vibrationPattern: 100, 1000, 100, 1000, 100
                tag: ph_ec_alert
            action: notify.mobile_app_s23ultra
  - data:
      message: >-
        Process complete - pH: {{ states('sensor.atlas_legacy_1_ph_res1') }},
        EC: {{ states('sensor.atlas_legacy_1_ec_res1') }}
      data:
        tag: tank_fill_process
    action: notify.mobile_app_s23ultra
  - choose:
      - conditions:
          - condition: time
            after: "06:00:00"
            before: "02:00:00"
        sequence:
          - target:
              entity_id: automation.f1_irrigation_irrigate_5_minutes_every_45_minutes
            action: automation.turn_on
            data: {}
          - data:
              message: Irrigation schedule resumed after tank fill (daytime)
              data:
                tag: tank_fill_process
            action: notify.mobile_app_s23ultra
    default:
      - data:
          message: Tank fill complete, but irrigation remains disabled (nighttime)
          data:
            tag: tank_fill_process
        action: notify.mobile_app_s23ultra
  - data:
      message: Tank fill and dose sequence complete - Starting cooldown period
      data:
        tag: tank_fill_process
    action: notify.mobile_app_s23ultra
  - action: input_boolean.turn_off
    metadata: {}
    data: {}
    target:
      entity_id: input_boolean.nutrient_dosing_active
  - delay:
      hours: 1
      minutes: 30
      seconds: 0
      milliseconds: 0
  - action: input_boolean.turn_on
    metadata: {}
    data: {}
    target:
      entity_id: input_boolean.nutrient_dosing_active
mode: single

```
