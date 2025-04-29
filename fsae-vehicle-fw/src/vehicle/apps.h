// Anteater Electric Racing, 2025

#pragma once

#include "bse.h"

typedef struct {
    float appsReading1_Percentage; // Percentage of pedal travel (0 to 1)
    float appsReading2_Percentage; // Percentage of pedal travel (0 to 1)
} APPSData;

void APPS_Init();
void APPS_UpdateData(uint32_t rawReading1, uint32_t rawReading2);
float APPS_GetAPPSReading();
APPSData APPS_GetAPPSReading_Separate();