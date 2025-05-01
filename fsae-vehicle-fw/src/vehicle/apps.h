// Anteater Electric Racing, 2025

#pragma once

#include "bse.h"

#include "utils/utils.h"

#define APPS_IMPLAUSABILITY_THRESHOLD 0.1            // 10%
#define APPS_BSE_PLAUSABILITY_TROTTLE_THRESHOLD 0.25 // 25%
#define APPS_BSE_PLAUSABILITY_BRAKE_THRESHOLD 50    // PSI

#define VOLTAGE_DIVIDER 2.0F
#define APPS_ADC_TO_VOLTAGE(x) ((x) * (LOGIC_LEVEL_V / 1027.0F)) * VOLTAGE_DIVIDER

#define APPS_3V3_PERCENTAGE(x) ((x) / 3.3F)
#define APPS_5V_PERCENTAGE(x) ((x) / 5.0F)

typedef struct {
    float appsReading1_Percentage; // Percentage of pedal travel (0 to 1)
    float appsReading2_Percentage; // Percentage of pedal travel (0 to 1)
} APPSData;

void APPS_Init();
void APPS_UpdateData(uint32_t rawReading1, uint32_t rawReading2);
float APPS_GetAPPSReading();
APPSData APPS_GetAPPSReading_Separate();