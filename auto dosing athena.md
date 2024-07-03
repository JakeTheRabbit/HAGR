# Helpers in configuration.yaml
```
input_number:
  ec_value:
    name: EC Value
    min: 0
    max: 4
    step: 0.1
    mode: box
  solution_volume:
    name: Solution Volume
    min: 0
    max: 500
    step: 1
    mode: box
  balance_dose:
    name: "Balance Dose Amount (ml)"
    min: 0
    max: 1000
    step: 1
  cleanse_dose:
    name: "Cleanse Dose Amount (ml)"
    min: 0
    max: 1000
    step: 1
  core_dose:
    name: "Core Dose Amount (ml)"
    min: 0
    max: 1000
    step: 1
  bloom_dose:
    name: "Bloom Dose Amount (ml)"
    min: 0
    max: 1000
    step: 1
    
input_boolean:
  trigger_balance:
    name: "Dose Balance"
  trigger_cleanse:
    name: "Dose Cleanse"
  trigger_core:
    name: "Dose Core"
  trigger_bloom:
    name: "Dose Bloom"
```
# Automations in automations.yaml
```
- alias: Dose Balance
  trigger:
    platform: state
    entity_id: input_boolean.trigger_balance
    to: 'on'
  action:
    - service: switch.turn_on
      entity_id: switch.peri_2
    - delay:
        seconds: '{{ (states(''input_number.balance_dose'') | float) / 0.75 | round(0) }}'
    - service: switch.turn_off
      entity_id: switch.peri_2
    - service: input_boolean.turn_off
      entity_id: input_boolean.trigger_balance
  id: 9698cdae9b6f49769c4078b8447a02bb

- alias: Dose Cleanse
  trigger:
    platform: state
    entity_id: input_boolean.trigger_cleanse
    to: 'on'
  action:
    - service: switch.turn_on
      entity_id: switch.peri_1
    - delay:
        seconds: '{{ (states(''input_number.cleanse_dose'') | float) / 0.75 | round(0) }}'
    - service: switch.turn_off
      entity_id: switch.peri_1
    - service: input_boolean.turn_off
      entity_id: input_boolean.trigger_cleanse
  id: d2640090e6e04efa8f2f2c67d7b9196a

- alias: Dose Core
  trigger:
    platform: state
    entity_id: input_boolean.trigger_core
    to: 'on'
  action:
    - service: switch.turn_on
      entity_id: switch.peri_3
    - delay:
        seconds: '{{ (states(''input_number.core_dose'') | float) / 0.75 | round(0) }}'
    - service: switch.turn_off
      entity_id: switch.peri_3
    - service: input_boolean.turn_off
      entity_id: input_boolean.trigger_core
  id: c8ece30d730548e381b1772e5dda4b91

- alias: Dose Bloom
  trigger:
    platform: state
    entity_id: input_boolean.trigger_bloom
    to: 'on'
  action:
    - service: switch.turn_on
      entity_id: switch.peri_4
    - delay:
        seconds: '{{ (states(''input_number.bloom_dose'') | float) / 0.75 | round(0) }}'
    - service: switch.turn_off
      entity_id: switch.peri_4
    - service: input_boolean.turn_off
      entity_id: input_boolean.trigger_bloom

```
