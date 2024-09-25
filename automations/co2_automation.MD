```mermaid
graph TD
    A[Start] -->|Trigger| B{Which Trigger?}
    
    B -->|CO₂ Sensor State Change<br>(co2_sensor_state_change)| C[CO₂ Control Logic]
    B -->|CO₂ Target Input Change<br>(co2_target_state_change)| C
    B -->|CO₂ Tolerance Input Change<br>(co2_tolerance_input_change)| C
    B -->|CO₂ On Time Trigger<br>(co2_on_time_trigger)| D[Daytime CO₂ Target Adjustment]
    B -->|CO₂ Off Time Trigger<br>(co2_off_time_trigger)| E[Nighttime CO₂ Target Adjustment]
    B -->|Low CO₂ Alert Trigger<br>(low_co2_alert_trigger)| F[Low CO₂ Alert]
    B -->|High CO₂ Alert Trigger<br>(high_co2_alert_trigger)| G[High CO₂ Alert (Primary Sensor)]
    B -->|High CO₂ Alert Trigger Secondary<br>(high_co2_alert_trigger_secondary)| H[High CO₂ Alert (Secondary Sensor)]
    
    %% Daytime CO₂ Target Adjustment
    D --> D1[Get Daytime Target Value]
    D1 --> D2{Is Target Value Valid?}
    D2 -->|Yes| D3[Set CO₂ Target Input Number]
    D2 -->|No| I[End]
    
    %% Nighttime CO₂ Target Adjustment
    E --> E1[Get Nighttime Target Value]
    E1 --> E2{Is Target Value Valid?}
    E2 -->|Yes| E3[Set CO₂ Target Input Number]
    E2 -->|No| I
    
    %% Low CO₂ Alert
    F --> F1{Is Current Time Within Alert Window?}
    F1 -->|Yes| F2{Is CO₂ Value Valid?}
    F1 -->|No| I
    F2 -->|Yes| F3[Send Low CO₂ Notification]
    F3 --> F4[Adjust Lights to Low Brightness]
    F2 -->|No| I
    
    %% High CO₂ Alert (Primary Sensor)
    G --> G1{Is CO₂ Level Valid?}
    G1 -->|Yes| G2[Send High CO₂ Notification]
    G2 --> G3[Turn Off CO₂ Control Switch]
    G3 --> G4[Create Persistent Notification]
    G4 --> G5[Send TTS Message]
    G1 -->|No| I
    
    %% High CO₂ Alert (Secondary Sensor)
    H --> H1{Is Secondary Sensor Defined?}
    H1 -->|Yes| H2{Is CO₂ Level Valid?}
    H1 -->|No| I
    H2 -->|Yes| H3[Send High CO₂ Notification]
    H3 --> H4[Turn Off CO₂ Control Switch]
    H4 --> H5[Create Persistent Notification]
    H5 --> H6[Send TTS Message]
    H2 -->|No| I
    
    %% CO₂ Control Logic
    C --> C1{Are CO₂ Values Valid?}
    C1 -->|Yes| C2{Is CO₂ Below Lower Limit<br>(co2_target - co2_tolerance)?}
    C1 -->|No| I
    C2 -->|Yes| C3{Is CO₂ Control Switch Off?}
    C3 -->|Yes| C4[Turn On CO₂ Control Switch]
    C3 -->|No| I
    C2 -->|No| C5{Is CO₂ Above Upper Limit<br>(co2_target + co2_tolerance)?}
    C5 -->|Yes| C6{Is CO₂ Control Switch On?}
    C6 -->|Yes| C7[Turn Off CO₂ Control Switch]
    C6 -->|No| I
    C5 -->|No| I
    
    I[End]



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
