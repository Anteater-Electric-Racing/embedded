// Anteater Electric Racing, 2025

#pragma once

#include "bse.h"

void APPS_Init();
void APPS_UpdateData(uint32_t rawReading1, uint32_t rawReading2);
float APPS_GetAPPSReading();
float APPS_GetAPPSReading1();
float APPS_GetAPPSReading2();
