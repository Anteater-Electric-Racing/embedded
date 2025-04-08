// Anteater Electric Racing, 2025

#include "bse.h"

typedef struct {
    float appsReading1;
    float appsReading2;
} APPSData;

static APPSData appsData;

void APPS_Init();
void checkAndHandleAPPSFault();
void checkAndHandlePlausibilityFault();
void APPS_UpdateData(APPSData *data);
float APPS_GetAPPSReading();
