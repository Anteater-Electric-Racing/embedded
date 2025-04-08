// Anteater Electric Racing, 2025

#define APPS_IMPLAUSABILITY_THRESHOLD 0.1
#define APPS_BSE_PLAUSABILITY_TROTTLE_THRESHOLD 0.25
#define APPS_BSE_PLAUSABILITY_BRAKE_THRESHOLD 0.1 // ???

#include <cmath>

#include "apps.h"

#include "vehicle/faults.h"

void APPS_Init() {
    appsData.appsReading1 = 0;
    appsData.appsReading2 = 0;
}

void checkAndHandleAPPSFault() {
    float difference = abs(appsData.appsReading1 - appsData.appsReading2);

    if (difference > APPS_IMPLAUSABILITY_THRESHOLD) {
        Faults_SetFault(FAULT_APPS);
    } else {
        Faults_ClearFault(FAULT_APPS);
    }
}

void checkAndHandlePlausibilityFault() {
    float BSEReading = BSE_GetBSEReading();

    if (APPS_GetAPPSReading() > APPS_BSE_PLAUSABILITY_TROTTLE_THRESHOLD &&
        BSEReading > APPS_BSE_PLAUSABILITY_BRAKE_THRESHOLD) {
        Faults_SetFault(FAULT_APPS_BRAKE_PLAUSIBILITY);
    } else {
        Faults_ClearFault(FAULT_APPS_BRAKE_PLAUSIBILITY);
    }
}

// TODO: confirm input data
void APPS_UpdateData(APPSData *data) {
    appsData.appsReading1 = data->appsReading1;
    appsData.appsReading2 = data->appsReading2;

    checkAndHandleAPPSFault();
}

float APPS_GetAPPSReading() {
    return (appsData.appsReading1 + appsData.appsReading2) / 2;
}
