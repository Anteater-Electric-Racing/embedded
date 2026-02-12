// Anteater Electric Racing, 2025

#pragma once

/*TODO Fixes:
 ANSI ESCAPE CODES LES GO


2/12 Morning (8am - 10:30am sprint)
    - Add only ONE testing flag (only debug flag)
    - DONE Move updateMotor into motorTask.



    - add checksum for BMS messages
    - Fix PCC packet structure (acc/Ts voltage as uint8s)
    - DONE Make threadMain() the GPIO spot
    - Better serial output (with everything. look into this online examples or
   tools or libs or vscode extension??

- LOOK AT THREAD STACK SIZES???

    - DONE APPS sensors for throttle control (variable MAX adjusted with
serial?) -- see if I can keep this flashed past startup?

    - Big thing will be current limiting and variable derating, this will take 1
hr to implement properly with testing features along the way


   ENSURE NO LOGIC Breaks, bare minum 10% should work.
*/

#define DEBUG_FLAG 0
#define HIMAC_FLAG 0
#define BMS_FLAG 0 // TO REMOVE

#define HIGH 1
#define LOW 0

#define THREAD_MAIN_STACK_SIZE 128
#define THREAD_MAIN_PRIORITY 1
#define THREAD_MOTOR_STACK_SIZE 128
#define THREAD_MOTOR_PRIORITY 4
#define THREAD_CAN_TELEMETRY_STACK_SIZE 512
#define THREAD_CAN_TELEMETRY_PRIORITY 1
#define THREAD_ADC_STACK_SIZE 128
#define THREAD_ADC_PRIORITY 8

#define WHEEL_SPEED_1_PIN 2
#define WHEEL_SPEED_2_PIN 3
#define RTM_BUTTON_PIN 4

#define LOGIC_LEVEL_V 3.3F
#define TIME_STEP 0.001F // 1ms time step

#define ADC_AVERAGING 1
#define ADC_RESOLUTION 12
#define ADC_MAX_VALUE ((1 << ADC_RESOLUTION) - 1)
#define TICKTYPE_FREQUENCY 1

#define ADC_VOLTAGE_DIVIDER 1.515151F

#define ADC_VALUE_TO_VOLTAGE(x)                                                \
    ((x) * (LOGIC_LEVEL_V * ADC_VOLTAGE_DIVIDER / ADC_MAX_VALUE))

#define APPS_FAULT_PERCENT_MIN .1
#define APPS_FAULT_PERCENT_MAX .9

#define APPS1_VOLTAGE_LEVEL 3.3
#define APPS2_VOLTAGE_LEVEL 5

#define APPS_RANGE_MIN_PERCENT .15
#define APPS_RANGE_MAX_PERCENT .85

#define APPS_3V3_MIN (APPS1_VOLTAGE_LEVEL * APPS_RANGE_MIN_PERCENT)
#define APPS_3V3_MAX (APPS1_VOLTAGE_LEVEL * APPS_RANGE_MAX_PERCENT)

#define APPS_5V_MIN (APPS2_VOLTAGE_LEVEL * APPS_RANGE_MIN_PERCENT)
#define APPS_5V_MAX (APPS2_VOLTAGE_LEVEL * APPS_RANGE_MAX_PERCENT)

/*     ANOOP TESTING FOR 20% HERE     */

// APPS 0-20% -> 0-100% scaling

// ADC values corresponding to physical 20% pedal)
#define APPS1_20PCT_ADC 784.0F
#define APPS2_20PCT_ADC 1150.0F

/**KZ Driving MAX (1+2) */
#define APPS1_FULL_PCT_ADC 1580.0F
#define APPS2_FULL_PCT_ADC 3680.0F

// Measured resting ADC (change these with actual findings this is just safe
// zone values)
/**KZ Driving MIN (1+2) */
#define APPS1_REST_ADC 4.75F
#define APPS2_REST_ADC 2797.0F

// Clamp helper
#define CLAMP(x, lo, hi) ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))
#define CLAMP01(x) CLAMP((x), 0.0F, 1.0F)

// Convert raw ADC -> commanded percent using only 0-20% physical range
#define APPS_ADC_TO_CMD_PERCENT(adc, rest_adc, adc_20)                         \
    CLAMP01(((float)(adc) - (float)(rest_adc)) /                               \
            ((float)(adc_20) - (float)(rest_adc)))

/*     END ANOOP TESTING FOR 20% HERE     */

#define APPS_3V3_FAULT_MIN (APPS1_VOLTAGE_LEVEL * APPS_FAULT_PERCENT_MIN)
#define APPS_3V3_FAULT_MAX (APPS1_VOLTAGE_LEVEL * APPS_FAULT_PERCENT_MAX)

#define APPS_5V_FAULT_MIN (APPS2_VOLTAGE_LEVEL * APPS_FAULT_PERCENT_MIN)
#define APPS_5V_FAULT_MAX (APPS2_VOLTAGE_LEVEL * APPS_FAULT_PERCENT_MAX)

#define APPS_FAULT_TIME_THRESHOLD_MS 100

#define APPS_IMPLAUSABILITY_THRESHOLD 0.1            // 10%
#define APPS_BSE_PLAUSABILITY_TROTTLE_THRESHOLD 0.15 // 15%
#define APPS_BSE_PLAUSABILITY_BRAKE_THRESHOLD                                  \
    0.50 // TODO: change back to PSI200    // IN VOLTS
#define APPS_BSE_PLAUSIBILITY_RESET_THRESHOLD 0.05 // 5%

#define BSE_VOLTAGE_DIVIDER 2.0F // TODO: Update with real value
#define BSE_ADC_VALUE_TO_VOLTAGE(x)                                            \
    (x * (LOGIC_LEVEL_V / ADC_MAX_VALUE)) *                                    \
        BSE_VOLTAGE_DIVIDER // ADC value to voltage conversion

#define BSE_VOLTAGE_TO_PSI(x) x // Voltage to PSI conversion

#define BRAKE_LIGHT_THRESHOLD 0.5F
#define BSE_LOWER_THRESHOLD 0.25F
#define BSE_UPPER_THRESHOLD 4.5F
#define BSE_IMPLAUSABILITY_THRESHOLD 0.1F

#define BSE_FAULT_TIME_THRESHOLD_MS 100

#define BSE_CUTOFF_HZ 100.0F

#define MOTOR_MAX_TORQUE 152.0F // TODO: Update with real value //used to be 260
#define CAPPED_MOTOR_TORQUE 50.0F

#define BATTERY_MAX_CURRENT_A 1.0F // TO CHANGE
#define BATTERY_MAX_REGEN_A 1.0F   // TO CHANGE

#define COMPUTE_ALPHA(CUTOFF_HZ)                                               \
    (1.0F / (1.0F + (1.0F / (2.0F * M_PI * CUTOFF_HZ)) / TIME_STEP))

#define LOWPASS_FILTER(NEW, OLD, ALPHA) OLD = ALPHA * NEW + (1.0F - ALPHA) * OLD

#define LINEAR_MAP(x, in_min, in_max, out_min, out_max)                        \
    ((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min)

#define CHANGE_ENDIANESS_16(x) (((x & 0xFF00) >> 8) | ((x & 0x00FF) << 8))

#define MOTOR_DIRECTION_STANDBY 0
#define MOTOR_DIRECTION_FORWARD 1
#define MOTOR_DIRECTION_BACKWARD 2
#define MOTOR_DIRECTION_ERROR 3

#define MAX_REGEN_TORQUE -9.0F // TODO: test with higher value regen
#define REGEN_BIAS 1           // Scale 0-1 of max regen torque
