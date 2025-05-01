
This directory is intended for PlatformIO Test Runner and project tests.

Unit Testing is a software testing method by which individual units of
source code, sets of one or more MCU program modules together with associated
control data, usage procedures, and operating procedures, are tested to
determine whether they are fit for use. Unit testing finds problems early
in the development cycle.

More information about PlatformIO Unit Testing:
- https://docs.platformio.org/en/latest/advanced/unit-testing/index.html


## APPS Tests

| Test Case                                  | Description                                     | Expected Behavior                        | Notes
|--------------------------------------------|-------------------------------------------------|------------------------------------------|-------------------------------------
| APPS Initialization                        | Ensure APPS_Init sets readings to 0             | No faults, values = 0                    | ✅ 
| APPS UpdateData                            | Ensure APPS_UpdateData correctly updates data   | No faults, values are correctly set      | ✅
| APPS Implausibility Fault                  | >10% disagreement between sensors               | Fault set: FAULT_APPS                    | ✅
| APPS + Brake Plausibility Fault            | Throttle >25%, Brake >200 PSI                   | Fault set: FAULT_APPS_BRAKE_PLAUSIBILITY | ✅
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

sanity
init
all clear

set faults
clear faults



## Motor Module Tests

init
motor state
clear fault state
get motor state