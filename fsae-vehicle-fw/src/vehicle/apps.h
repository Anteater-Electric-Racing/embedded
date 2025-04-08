// Anteater Electric Racing, 2025

#include "bse.h"

typedef struct {
    float appsReading1_Percentage; // 0-1
    float appsReading2_Percentage; // 0-1
} APPSData;

static APPSData appsData;

void APPS_Init();
void checkAndHandleAPPSFault();
void checkAndHandlePlausibilityFault();
void APPS_UpdateData(uint32_t rawReading1, uint32_t rawReading2);
float APPS_GetAPPSReading();
