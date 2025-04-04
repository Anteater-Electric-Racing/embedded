// Anteater Electric Racing, 2025

#define APPS_IMPLAUSABILITY_THRESHOLD 0.1
#define APPS_BSE_PLAUSABILITY_TROTTLE_THRESHOLD 0.25
#define APPS_BSE_PLAUSABILITY_BRAKE_THRESHOLD 0.1 // ???

#include <cmath>

#include "apps.h"

#include "vehicle/faults.h"

APPS::APPS() {
    _APPSdata.appsReading1 = 0;
    _APPSdata.appsReading2 = 0;
}

APPS::~APPS() {}

void APPS::checkAndHandleAPPSFault() {
    float difference = abs(_APPSdata.appsReading1 - _APPSdata.appsReading2);

    if (difference > APPS_IMPLAUSABILITY_THRESHOLD) {
        Faults_SetFault(FAULT_APPS);
    } else {
        Faults_ClearFault(FAULT_APPS);
    }
}

void APPS::checkAndHandlePlausibilityFault(BSE *bse) {
    float BSEReading = bse->BSE_GetBSEReading();

    if (APPS_GetAPPSReading() > APPS_BSE_PLAUSABILITY_TROTTLE_THRESHOLD &&
        BSEReading > APPS_BSE_PLAUSABILITY_BRAKE_THRESHOLD) {
        Faults_SetFault(FAULT_APPS_BRAKE_PLAUSIBILITY);
    } else {
        Faults_ClearFault(FAULT_APPS_BRAKE_PLAUSIBILITY);
    }
}

// TODO: confirm input data
void APPS::APPS_UpdateData(APPSData *data) {
    _APPSdata.appsReading1 = data->appsReading1;
    _APPSdata.appsReading2 = data->appsReading2;

    checkAndHandleAPPSFault();
}

float APPS::APPS_GetAPPSReading() {
    return (_APPSdata.appsReading1 + _APPSdata.appsReading2) / 2;
}
