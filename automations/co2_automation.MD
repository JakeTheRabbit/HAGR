
```mermaid
graph TD
    Start[Start] -->|Trigger| WhichTrigger{Which Trigger?}

    WhichTrigger -->|"CO2 Sensor State Change<br>co2_sensor_state_change"| CO2ControlLogic[CO2 Control Logic]
    WhichTrigger -->|"CO2 Target Input Change<br>co2_target_state_change"| CO2ControlLogic
    WhichTrigger -->|"CO2 Tolerance Input Change<br>co2_tolerance_state_change"| CO2ControlLogic
    WhichTrigger -->|"CO2 On Time Trigger<br>co2_on_time_trigger"| DaytimeAdjustment[Daytime CO2 Target Adjustment]
    WhichTrigger -->|"CO2 Off Time Trigger<br>co2_off_time_trigger"| NighttimeAdjustment[Nighttime CO2 Target Adjustment]
    WhichTrigger -->|"Low CO2 Alert Trigger<br>low_co2_alert_trigger"| LowCO2Alert[Low CO2 Alert]
    WhichTrigger -->|"High CO2 Alert Trigger<br>high_co2_alert_trigger"| HighCO2AlertPrimary[High CO2 Alert Primary Sensor]
    WhichTrigger -->|"High CO2 Alert Trigger Secondary<br>high_co2_alert_trigger_secondary"| HighCO2AlertSecondary[High CO2 Alert Secondary Sensor]

    %% Daytime CO2 Target Adjustment
    DaytimeAdjustment --> GetDaytimeTarget[Get Daytime Target Value]
    GetDaytimeTarget --> ValidateDaytimeTarget{Is Target Value Valid?}
    ValidateDaytimeTarget -->|Yes| SetDaytimeTarget[Set CO2 Target Input Number]
    ValidateDaytimeTarget -->|No| End[End]

    %% Nighttime CO2 Target Adjustment
    NighttimeAdjustment --> GetNighttimeTarget[Get Nighttime Target Value]
    GetNighttimeTarget --> ValidateNighttimeTarget{Is Target Value Valid?}
    ValidateNighttimeTarget -->|Yes| SetNighttimeTarget[Set CO2 Target Input Number]
    ValidateNighttimeTarget -->|No| End

    %% Low CO2 Alert
    LowCO2Alert --> CheckLowCO2Time{Is Current Time Within Alert Window?}
    CheckLowCO2Time -->|Yes| ValidateLowCO2Value{Is CO2 Value Valid?}
    CheckLowCO2Time -->|No| End
    ValidateLowCO2Value -->|Yes| SendLowCO2Notification[Send Low CO2 Notification]
    SendLowCO2Notification --> AdjustLightsLow[Adjust Lights to Low Brightness]
    ValidateLowCO2Value -->|No| End

    %% High CO2 Alert (Primary Sensor)
    HighCO2AlertPrimary --> ValidateHighCO2Primary{Is CO2 Level Valid?}
    ValidateHighCO2Primary -->|Yes| SendHighCO2NotificationPrimary[Send High CO2 Notification]
    SendHighCO2NotificationPrimary --> TurnOffControlSwitchPrimary[Turn Off CO2 Control Switch]
    TurnOffControlSwitchPrimary --> CreatePersistentNotificationPrimary[Create Persistent Notification]
    CreatePersistentNotificationPrimary --> SendTTSPMessagePrimary[Send TTS Message]
    ValidateHighCO2Primary -->|No| End

    %% High CO2 Alert (Secondary Sensor)
    HighCO2AlertSecondary --> CheckSecondarySensor{Is Secondary Sensor Defined?}
    CheckSecondarySensor -->|Yes| ValidateHighCO2Secondary{Is CO2 Level Valid?}
    CheckSecondarySensor -->|No| End
    ValidateHighCO2Secondary -->|Yes| SendHighCO2NotificationSecondary[Send High CO2 Notification]
    SendHighCO2NotificationSecondary --> TurnOffControlSwitchSecondary[Turn Off CO2 Control Switch]
    TurnOffControlSwitchSecondary --> CreatePersistentNotificationSecondary[Create Persistent Notification]
    CreatePersistentNotificationSecondary --> SendTTSPMessageSecondary[Send TTS Message]
    ValidateHighCO2Secondary -->|No| End

    %% CO2 Control Logic
    CO2ControlLogic --> ValidateCO2Values{Are CO2 Values Valid?}
    ValidateCO2Values -->|Yes| CheckCO2LowerLimit{"Is CO2 Below Lower Limit<br>(co2_target - co2_tolerance)?"}
    ValidateCO2Values -->|No| End
    CheckCO2LowerLimit -->|Yes| CheckControlSwitchOff{Is CO2 Control Switch Off?}
    CheckControlSwitchOff -->|Yes| TurnOnControlSwitch[Turn On CO2 Control Switch]
    CheckControlSwitchOff -->|No| End
    CheckCO2LowerLimit -->|No| CheckCO2UpperLimit{"Is CO2 Above Upper Limit<br>(co2_target + co2_tolerance)?"}
    CheckCO2UpperLimit -->|Yes| CheckControlSwitchOn{Is CO2 Control Switch On?}
    CheckCO2UpperLimit -->|No| End
    CheckControlSwitchOn -->|Yes| TurnOffControlSwitch[Turn Off CO2 Control Switch]
    CheckControlSwitchOn -->|No| End
    CheckCO2UpperLimit -->|No| End

    End[End]
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
