// Anteater Electric Racing, 2025

#pragma once

#define LOGIC_LEVEL_V 3.3F
#define TIME_STEP 0.001F // 1ms time step

#define ADC_RESOLUTION 12
#define ADC_MAX_VALUE ((1 << ADC_RESOLUTION) - 1)

#define ADC_VOLTAGE_DIVIDER 2.0F
#define ADC_VALUE_TO_VOLTAGE(x) ((x) * (LOGIC_LEVEL_V / ADC_MAX_VALUE)) * ADC_VOLTAGE_DIVIDER

#define APPS_3V3_FAULT_MIN 0.005F
#define APPS_3V3_FAULT_MAX 1.8F

#define APPS_3V3_MIN 0.1F
#define APPS_3V3_MAX 1.7F

#define COMPUTE_ALPHA(CUTOFF_HZ) \
    (1.0F / (1.0F + (1.0F / (2.0F * M_PI * CUTOFF_HZ)) / TIME_STEP))

#define LOWPASS_FILTER(NEW, OLD, ALPHA) \
    OLD = ALPHA * NEW + (1.0F - ALPHA) * OLD

#define LINEAR_MAP(x, in_min, in_max, out_min, out_max) \
    ((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min)
