```mermaid
graph TD
    A[Start] -->|Trigger| B{Which Trigger?}

    B -->|"CO2 Sensor State Change\nco2_sensor_state_change"| C[CO2 Control Logic]
    B -->|"CO2 Target Input Change\nco2_target_state_change"| C
    B -->|"CO2 Tolerance Input Change\nco2_tolerance_state_change"| C
    B -->|"CO2 On Time Trigger\nco2_on_time_trigger"| D[Daytime CO2 Target Adjustment]
    B -->|"CO2 Off Time Trigger\nco2_off_time_trigger"| E[Nighttime CO2 Target Adjustment]
    B -->|"Low CO2 Alert Trigger\nlow_co2_alert_trigger"| F[Low CO2 Alert]
    B -->|"High CO2 Alert Trigger\nhigh_co2_alert_trigger"| G[High CO2 Alert (Primary Sensor)]
    B -->|"High CO2 Alert Trigger Secondary\nhigh_co2_alert_trigger_secondary"| H[High CO2 Alert (Secondary Sensor)]

    %% Daytime CO2 Target Adjustment
    D --> D1[Get Daytime Target Value]
    D1 --> D2{Is Target Value Valid?}
    D2 -->|Yes| D3[Set CO2 Target Input Number]
    D2 -->|No| I[End]

    %% Nighttime CO2 Target Adjustment
    E --> E1[Get Nighttime Target Value]
    E1 --> E2{Is Target Value Valid?}
    E2 -->|Yes| E3[Set CO2 Target Input Number]
    E2 -->|No| I

    %% Low CO2 Alert
    F --> F1{Is Current Time Within Alert Window?}
    F1 -->|Yes| F2{Is CO2 Value Valid?}
    F1 -->|No| I
    F2 -->|Yes| F3[Send Low CO2 Notification]
    F3 --> F4[Adjust Lights to Low Brightness]
    F2 -->|No| I

    %% High CO2 Alert (Primary Sensor)
    G --> G1{Is CO2 Level Valid?}
    G1 -->|Yes| G2[Send High CO2 Notification]
    G2 --> G3[Turn Off CO2 Control Switch]
    G3 --> G4[Create Persistent Notification]
    G4 --> G5[Send TTS Message]
    G1 -->|No| I

    %% High CO2 Alert (Secondary Sensor)
    H --> H1{Is Secondary Sensor Defined?}
    H1 -->|Yes| H2{Is CO2 Level Valid?}
    H1 -->|No| I
    H2 -->|Yes| H3[Send High CO2 Notification]
    H3 --> H4[Turn Off CO2 Control Switch]
    H4 --> H5[Create Persistent Notification]
    H5 --> H6[Send TTS Message]
    H2 -->|No| I

    %% CO2 Control Logic
    C --> C1{Are CO2 Values Valid?}
    C1 -->|Yes| C2{"Is CO2 Below Lower Limit\n(co2_target - co2_tolerance)?"}
    C1 -->|No| I
    C2 -->|Yes| C3{Is CO2 Control Switch Off?}
    C3 -->|Yes| C4[Turn On CO2 Control Switch]
    C3 -->|No| I
    C2 -->|No| C5{"Is CO2 Above Upper Limit\n(co2_target + co2_tolerance)?"}
    C5 -->|Yes| C6{Is CO2 Control Switch On?}
    C6 -->|Yes| C7[Turn Off CO2 Control Switch]
    C6 -->|No| I
    C5 -->|No| I

    I[End]

```



### **Explanation of the Diagram:**

- **Start:** The automation begins when any of the specified triggers occur.
  
- **Trigger Decision Point (B):** Determines which trigger activated the automation and directs the flow accordingly.

#### **CO₂ Target Adjustments:**

- **Daytime CO₂ Target Adjustment (D):**
  - Gets the daytime target value.
  - Checks if the value is valid.
  - Sets the CO₂ target input number if valid.

- **Nighttime CO₂ Target Adjustment (E):**
  - Similar to daytime adjustment but uses the nighttime target value.

#### **Low CO₂ Alert (F):**

- Checks if the current time is within the low CO₂ alert window.
- Validates the CO₂ sensor value.
- If valid, sends a low CO₂ notification and adjusts lights to the specified low brightness.

#### **High CO₂ Alert (Primary Sensor) (G):**

- Validates the CO₂ level from the primary sensor.
- If valid, performs the following actions:
  - Sends a high CO₂ notification with CO₂ levels.
  - Turns off the CO₂ control switch.
  - Creates a persistent notification.
  - Sends a TTS message.

#### **High CO₂ Alert (Secondary Sensor) (H):**

- Checks if the secondary sensor is defined.
- Validates the CO₂ level from the secondary sensor.
- If valid, performs similar actions as the primary sensor alert.

#### **CO₂ Control Logic (C):**

- Validates the CO₂ sensor values and input numbers.
- Checks if the CO₂ level is below the lower limit (target minus tolerance):
  - If yes, and the CO₂ control switch is off, turns on the switch.
- Checks if the CO₂ level is above the upper limit (target plus tolerance):
  - If yes, and the CO₂ control switch is on, turns off the switch.

### **Notes:**

- **Validation Steps:** At multiple points, the automation checks if values are valid (not `None`, `unknown`, or `unavailable`) before proceeding.
  
- **End Nodes (I):** The automation ends if any conditions are not met or after actions are executed.

- **Sequence Flow:** The diagram illustrates the flow of logic based on the triggers and conditions, helping to visualize how the automation processes each event.

### **How to Use the Diagram:**

- **Understanding Triggers:** Use the diagram to see what happens when each trigger occurs.

- **Following the Logic:** Trace the path from the trigger through conditions and actions to understand the automation's behavior.

- **Identifying Conditions:** The decision points (diamonds) show where the automation checks for specific conditions before proceeding.

- **Actions:** Rectangular nodes represent actions taken by the automation.

### **Additional Considerations:**

- **Customizing the Automation:**
  - You can adjust thresholds, durations, and targets to fit your needs.
  - Ensure that any changes maintain the logical flow depicted in the diagram.

- **Testing:**
  - Use the diagram as a reference when testing each part of the automation.
  - Confirm that triggers and actions occur as expected.
