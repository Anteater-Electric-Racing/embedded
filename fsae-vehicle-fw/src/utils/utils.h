// Anteater Electric Racing, 2025

#pragma once

#define WHEEL_SPEED_1_PIN 2
#define WHEEL_SPEED_2_PIN 3
#define RTM_BUTTON_PIN 4

#define LOGIC_LEVEL_V 3.3F
#define TIME_STEP 0.001F // 1ms time step

#define ADC_RESOLUTION 12
#define ADC_MAX_VALUE ((1 << ADC_RESOLUTION) - 1)

#define ADC_VOLTAGE_DIVIDER 2.0F
#define ADC_VALUE_TO_VOLTAGE(x) ((x) * (LOGIC_LEVEL_V / ADC_MAX_VALUE)) * ADC_VOLTAGE_DIVIDER

#define APPS_FAULT_PERCENT_MIN .1
#define APPS_FAULT_PERCENT_MAX .9

#define APPS_3V3_MIN 0.37F
#define APPS_3V3_MAX 1.7F

#define APPS_5V_MIN 0.5F
#define APPS_5V_MAX 4.5F

#define APPS_IMPLAUSABILITY_THRESHOLD 0.1            // 10%
#define APPS_BSE_PLAUSABILITY_TROTTLE_THRESHOLD 0.25 // 25%
#define APPS_BSE_PLAUSABILITY_BRAKE_THRESHOLD 200    // PSI
#define APPS_BSE_PLAUSIBILITY_RESET_THRESHOLD 0.05 // 5%

#define BSE_VOLTAGE_DIVIDER 2.0F // TODO: Update with real value
#define BSE_ADC_VALUE_TO_VOLTAGE(x) (x * (LOGIC_LEVEL_V / ADC_MAX_VALUE)) * BSE_VOLTAGE_DIVIDER // ADC value to voltage conversion

#define BSE_VOLTAGE_TO_PSI(x) x // Voltage to PSI conversion

#define BSE_LOWER_THRESHOLD 0.5F
#define BSE_UPPER_THRESHOLD 4.5F
#define BSE_IMPLAUSABILITY_THRESHOLD 0.1F

#define BSE_CUTOFF_HZ 100.0F

#define MOTOR_MAX_TORQUE 250.0F // TODO: Update with real value

#define COMPUTE_ALPHA(CUTOFF_HZ) \
    (1.0F / (1.0F + (1.0F / (2.0F * M_PI * CUTOFF_HZ)) / TIME_STEP))

#define LOWPASS_FILTER(NEW, OLD, ALPHA) \
    OLD = ALPHA * NEW + (1.0F - ALPHA) * OLD

#define LINEAR_MAP(x, in_min, in_max, out_min, out_max) \
    ((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min)
