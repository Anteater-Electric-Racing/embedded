# Firmware Unit Testing

## APPS Tests

| Test Case                                  | Description                                     | Expected Behavior                        | Notes
|--------------------------------------------|-------------------------------------------------|------------------------------------------|-------------------------------------
| APPS Initialization                        | Ensure APPS_Init sets readings to 0             | No faults, values = 0                    | ✅ 
| APPS UpdateData                            | Ensure APPS_UpdateData correctly updates data   | No faults, values are correctly set      | ✅
| APPS Implausibility Fault                  | >10% disagreement between sensors               | Fault set: FAULT_APPS                    | ✅
| APPS + Brake Plausibility Fault            | Throttle >25%, Brake >200 PSI                   | Fault set: FAULT_APPS_BRAKE_PLAUSIBILITY | ✅
| APPS Voltage Range Fault                   | Voltage outside (0.5V - 4.5V) (0.17V - 1.8V)    | Fault set: FAULT_APPS_VOLTAGE            |
| APPS ADC to Voltage                        | Convert ADC to real-world voltage               | Accurate voltage conversion              | unsure if accurate
| APPS Low Pass Filter                       | Apply LPF to noisy input                        | Smoothed output                          | unsure how to test

## BSE Tests

| Test Case                                  | Description                                     | Expected Behavior                        | Notes
|--------------------------------------------|-------------------------------------------------|------------------------------------------|-------------------------------------
| BSE Initialization                         | Ensure BSE_Init sets readings to 0              | No faults, values = 0                    | ✅
| BSE Voltage Range Fault                    | Voltage outside 0.5V - 4.5V                     | Fault set: FAULT_BSE                     | needs testing with actual sensor
| BSE ADC to Voltage                         | Convert ADC to real-world voltage               | Accurate voltage conversion              | unsure if accurate
| BSE Voltage to PSI                         | Convert voltage to PSI                          | Accurate PSI output                      | need to confirm conversion
| BSE Implausibility Fault (optional)        | >10% PSI disagreement between front/rear        | Fault set: FAULT_BSE_IMPLAUSIBILITY (?)  | ✅ - still taking max of both sensors as reading
| BSE Low Pass Filter                        | Apply LPF to noisy input                        | Smoothed output                          | unsure how to test

## Fault Module Tests

| Test Case                                  | Description                                     | Expected Behavior                        | Notes
|--------------------------------------------|-------------------------------------------------|------------------------------------------|-------------------------------------
| Fault Module Init                          | Ensure all faults are false on init             | No faults active on init                 | ✅ 
| Faults All clear                           | Ensure Faults_CheckAllClear works               | Always true when no faults               | ✅
| Faults Set Faults                          | Ensure faults are set correctly                 | Faults set                               | ✅ 
| Faults Clear Faults                        | Ensure faults are cleared correctly             | Faults clear                             | ✅ 



## Motor Module Tests

| Test Case                                  | Description                                     | Expected Behavior                        | Notes
|--------------------------------------------|-------------------------------------------------|------------------------------------------|-------------------------------------
| Motor Module Init                          | Sanity check motor state starts as OFF          | No faults active on init                 | ✅ 
| Motor Update State (All States)            | Update motor state based on update logic        | Always true when no faults               | ✅
| Motor Torque Demand (w/ Fault)             | Hold correct motor demand + during motor fault  | Faults set                               | ✅ 
| Set Motor Fault State                      | Ensure faults are cleared correctly             | Faults clear                             | ✅ 