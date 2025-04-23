# Home Assistant Advanced Crop Steering Package

This package implements a dynamic, four-phase crop steering system for hydroponic setups within Home Assistant. It aims to optimize irrigation based on Volumetric Water Content (VWC), Electrical Conductivity (EC), time, and user-defined growth strategies (Vegetative vs. Generative).

## Core Principles

Crop steering involves manipulating irrigation frequency and volume to guide plant growth towards either vegetative (leafy growth) or generative (flowering/fruiting) stages. This package automates this process using sensor feedback and timed phases:

1.  **Dryback:** Allowing the substrate VWC to drop encourages root growth and can stress the plant towards generative responses. This package manages dryback targets differently depending on the phase and selected steering mode.
2.  **Irrigation Phases (P0-P3):**
    *   **P0 (Pre-Irrigation Dry Back):** Starts at "lights on". The system waits for the VWC to drop to a specific dryback target (`cs_p0_veg_dryback_target` or `cs_p0_gen_dryback_target`) or until a maximum wait time (`cs_p0_max_wait_time`) is reached before initiating the first irrigation cycle (transitioning to P1). This initial dry period encourages root exploration.
    *   **P1 (Ramp-Up):** Begins after the P0 dryback. This phase aims to gradually rehydrate the substrate to a target VWC (`cs_p1_target_vwc`). It does this by delivering irrigation "shots" at regular intervals (`cs_p1_time_between_shots`). The size of each shot starts small (`cs_p1_initial_shot_size_percent`) and increases with each subsequent shot (`cs_p1_shot_size_increment_percent`) until a maximum shot size (`cs_p1_max_shot_size_percent`) is reached. This prevents oversaturation while ensuring the core substrate gets adequately moistened. Transition to P2 occurs when a maximum shot count (`cs_p1_max_shots`) is reached, the target VWC is met, or if EC drops below the flush target (`cs_ec_target_flush`) indicating potential salt buildup needing flushing (requires minimum shots `cs_p1_min_shots` and target VWC `cs_p1_target_vwc` also met).
    *   **P2 (Maintenance):** The main daytime irrigation phase. It aims to maintain VWC near field capacity while managing EC. Irrigation is triggered when the average VWC (`sensor.cs_avg_vwc`) drops below an EC-adjusted threshold (`sensor.cs_p2_vwc_threshold_ec_adjusted`). If the measured EC (`sensor.cs_avg_ec`) is higher than desired relative to the target (`sensor.cs_current_ec_target`), the VWC threshold is raised slightly to trigger irrigation sooner (promoting leaching). If EC is lower than desired, the threshold is lowered to irrigate less frequently. Irrigation stops once the calculated shot duration (`sensor.cs_p2_shot_duration_seconds`) is complete or if VWC exceeds the substrate field capacity (`input_number.cs_substrate_field_capacity`).
    *   **P3 (Overnight Dry Back):** Starts a calculated time before "lights off" (`sensor.cs_p3_start_time_calculated`, based on `cs_p3_veg_last_irrigation` or `cs_p3_gen_last_irrigation`). All regular irrigation stops, allowing the substrate VWC to decrease overnight. An emergency irrigation (`sensor.cs_p3_emergency_shot_duration_seconds`) can trigger if VWC drops below a critical threshold (`input_number.cs_p3_emergency_vwc_threshold`).
3.  **Vegetative vs. Generative Modes:** Selecting the mode via `input_select.cs_steering_mode` adjusts key parameters (like dryback targets and P3 timing) to favor either vegetative or generative growth based on pre-defined values in `crop_steering_variables.yaml`.

## Package Structure & File Links

This package uses Home Assistant's `packages` feature to group all related configurations. The main file, `crop_steering_package.yaml`, acts as the entry point and uses `!include` tags to load configurations from other files within this directory.

*   **`crop_steering_package.yaml`:**
    *   **Purpose:** Main package file loaded by Home Assistant.
    *   **Links:**
        *   Includes `input_select`, `input_number`, `input_datetime` definitions from `crop_steering_variables.yaml`.
        *   Includes `template` sensor definitions by linking to `crop_steering_improved_sensors.yaml` and `crop_steering_aggregation_sensors.yaml`.
        *   Includes `automation` definitions from `crop_steering_improved_automations.yaml`.

*   **`crop_steering_variables.yaml`:**
    *   **Purpose:** Defines all user-configurable parameters (inputs). This allows tuning the system via the Home Assistant UI or by editing this file directly.
    *   **Links:** Included by `crop_steering_package.yaml`. Referenced by templates in `_improved_sensors.yaml` and `_improved_automations.yaml`. **This is the primary file for user tuning.**

*   **`crop_steering_improved_sensors.yaml`:**
    *   **Purpose:** Defines calculated sensors using the `template` integration. These sensors perform calculations needed by the automations (e.g., shot durations, EC ratio, dynamic thresholds, status descriptions).
    *   **Links:** Included by `crop_steering_package.yaml` under the `template:` key. References `input_*` helpers from `_variables.yaml` and aggregation sensors from `_aggregation_sensors.yaml`.

*   **`crop_steering_aggregation_sensors.yaml`:**
    *   **Purpose:** Defines template sensors that aggregate data from your *actual* physical sensors into the common entities used by the rest of the package (e.g., `sensor.cs_avg_vwc`, `sensor.cs_avg_ec`, `sensor.cs_min_vwc`, `sensor.cs_max_vwc`).
    *   **Links:** Included by `crop_steering_package.yaml` under the `template:` key. References your specific physical sensor entity IDs. **Requires user editing to map to physical sensors.**

*   **`crop_steering_improved_automations.yaml`:**
    *   **Purpose:** Contains all the automation logic that controls the phase transitions and triggers irrigation based on time, sensor states, and input helper values.
    *   **Links:** Included by `crop_steering_package.yaml` under the `automation:` key. References `input_*` helpers from `_variables.yaml` and `template` sensors from `_improved_sensors.yaml` and `_aggregation_sensors.yaml`. **Requires user editing for pump and notification service calls.**

*   **`README.md`:**
    *   **Purpose:** This documentation file.

## Installation

1.  **Copy Folder:** Place the entire `CropSteering` folder into your Home Assistant `packages` directory (e.g., `<config_dir>/packages/CropSteering/`). If you don't have a `packages` directory, create one.
2.  **Enable Packages:** Ensure your main `configuration.yaml` file includes the packages directory:
    ```yaml
    homeassistant:
      packages: !include_dir_named packages
    ```
    *(If you already have a `packages:` entry, ensure it points to the correct directory.)*
3.  **Restart Home Assistant:** Restart Home Assistant to load the new package.

## Configuration

**CRITICAL:** You MUST configure the package to match your specific hardware and preferences.

1.  **Map Your Sensors (`crop_steering_aggregation_sensors.yaml`):**
    *   Open `CropSteering/crop_steering_aggregation_sensors.yaml`.
    *   **Average Sensors:** Update the `state` templates for `cs_avg_vwc` and `cs_avg_ec` to point to *your* existing sensors that provide reliable average VWC and EC readings for the entire zone being controlled.
        ```yaml
        # Example:
        - name: "cs_avg_vwc"
          unique_id: cs_avg_vwc
          state: "{{ states('sensor.YOUR_AVERAGE_VWC_SENSOR_ID') | float(0) }}"
          # ... rest of config ...

        - name: "cs_avg_ec"
          unique_id: cs_avg_ec
          state: "{{ states('sensor.YOUR_AVERAGE_EC_SENSOR_ID') | float(0) }}"
          # ... rest of config ...
        ```
    *   **Min/Max VWC Sensors:** Update the list of sensor IDs within the `state` templates for `cs_min_vwc` and `cs_max_vwc` to include *all* your individual VWC sensors for the zone.
        ```yaml
        # Example:
        - name: "cs_min_vwc"
          unique_id: cs_min_vwc
          state: >
            {% set values = [
              states('sensor.YOUR_VWC_SENSOR_1') | float(999),
              states('sensor.YOUR_VWC_SENSOR_2') | float(999),
              # ... add all your VWC sensors ...
            ] %}
            # ... rest of template ...
        ```

2.  **Set Pump & Notification Entities (`crop_steering_improved_automations.yaml`):**
    *   Open `CropSteering/crop_steering_improved_automations.yaml`.
    *   **Search and Replace:** Find all instances of `switch.cs_irrigation_pump` and replace it with the actual entity ID of your main irrigation pump switch.
    *   **Search and Replace:** Find all instances of `notify.mobile_app_notify` and replace it with the correct service call for your desired notification platform (e.g., `notify.mobile_app_your_phone_name`, `notify.pushover`).

3.  **Tune Parameters (`crop_steering_variables.yaml`):**
    *   Open `CropSteering/crop_steering_variables.yaml`.
    *   Review and adjust all the `input_number` values to match your setup and desired steering strategy. Key sections include:
        *   `IRRIGATION SETTINGS`: Dripper flow rate.
        *   `P0 Phase settings`: Dryback targets, wait times.
        *   `P1 Phase settings`: Shot sizes, increments, timing, targets, min/max shots.
        *   `P2 Phase settings`: Shot size, VWC threshold, EC adjustment parameters.
        *   `P3 Phase settings`: Last irrigation timings, emergency thresholds/shot sizes.
        *   `EC TARGET SETTINGS`: Target EC for each phase in both Veg/Gen modes.
        *   `LIGHT SCHEDULE SETTINGS`: Your lights on/off times (used by sensors).
        *   `SUBSTRATE SETTINGS`: Volume, field capacity, saturation, critical VWC, etc.
    *   Adjust `input_datetime` values for `cs_lights_on_time` and `cs_lights_off_time`.
    *   Set the default `input_select` options if desired (`cs_steering_mode`, `cs_aggregation_method`).

4.  **Restart Home Assistant Again:** After making configuration changes, restart Home Assistant to apply them.

## Usage

1.  **Select Steering Mode:** Use the `input_select.cs_steering_mode` helper (default name: "Growth Steering Mode") in the Home Assistant UI to choose between "Vegetative" and "Generative" modes. This adjusts various target parameters automatically.
2.  **Monitor:** Observe the `input_select.cs_crop_steering_phase` (default name: "Crop Steering Phase") to see the current operational phase. Other sensors like `sensor.cs_current_phase_description`, `sensor.cs_irrigation_status`, `sensor.cs_avg_vwc`, `sensor.cs_avg_ec`, and the shot counters (`input_number.cs_pX_shot_count`) provide detailed status information.
3.  **Adjust Inputs:** Fine-tune parameters in `crop_steering_variables.yaml` (or via the UI helpers) as needed based on plant response and environmental conditions.

## Troubleshooting

*   **Check Logs:** After restarting Home Assistant, always check `Settings` -> `System` -> `Logs` for any YAML errors or issues related to entities starting with `cs_`.
*   **Verify Entities:** Go to `Developer Tools` -> `States` and filter by `cs_` to ensure all sensors, automations, and input helpers from the package have been created and have valid states. Pay close attention to the aggregated sensors (`cs_avg_vwc`, `cs_avg_ec`, `cs_min_vwc`, `cs_max_vwc`).
*   **Check Underlying Sensors:** Ensure the raw sensors you mapped in `crop_steering_aggregation_sensors.yaml` are providing valid, numeric data.
*   **Template Errors:** If sensors have `unavailable` or unexpected states, check their templates for errors using `Developer Tools` -> `Template`. Ensure all referenced input helpers exist and have valid values.
