// Anteater Electric Racing, 2025

#define APPS_IMPLAUSABILITY_THRESHOLD 0.1
#define APPS_BSE_PLAUSABILITY_TROTTLE_THRESHOLD 0.25
#define APPS_BSE_PLAUSABILITY_BRAKE_THRESHOLD 0.1   // ???

#define BSE_LOWER_THRESHOLD 0.5
#define BSE_UPPER_THRESHOLD 4.5
#define BSE_IMPLAUSABILITY_THRESHOLD 0.1

#include <cmath>

#include "bse.h"

#include "vehicle/faults.h"

APPSBSEData data;

void checkAndHandleAPPSFault() {
    float difference = abs(data.appsData.appsReading1 - data.appsData.appsReading2);
    
    if (difference > APPS_IMPLAUSABILITY_THRESHOLD) {
        Faults_SetFault(FAULT_APPS);
    } else {
        // is it ok to always call clear fault
        Faults_ClearFault(FAULT_APPS);
    }
}

void checkAndHandlePlausibilityFault() {
    float BSEReading = BSE_GetBSEReading();
    
    if (APPS_GetAPPSReading() > APPS_BSE_PLAUSABILITY_TROTTLE_THRESHOLD && BSEReading > APPS_BSE_PLAUSABILITY_BRAKE_THRESHOLD) {
        Faults_SetFault(FAULT_APPS_BRAKE_PLAUSIBILITY);
    } else {
        Faults_ClearFault(FAULT_APPS_BRAKE_PLAUSIBILITY);
    }
}

void APPS_UpdateData(APPSData* appsData) {
    data.appsData.appsReading1 = appsData->appsReading1;
    data.appsData.appsReading2 = appsData->appsReading2;

    checkAndHandleAPPSFault();
    checkAndHandlePlausibilityFault();
}

float APPS_GetAPPSReading() {
    return (data.appsData.appsReading1 + data.appsData.appsReading2) / 2;
}





void checkAndHandleBSEFault() {
    if (data.bseData.bseReading1 < BSE_LOWER_THRESHOLD || data.bseData.bseReading1 > BSE_UPPER_THRESHOLD || data.bseData.bseReading2 < BSE_LOWER_THRESHOLD || data.bseData.bseReading2 > BSE_UPPER_THRESHOLD) {
        Faults_SetFault(FAULT_BSE);
    } else {
        Faults_ClearFault(FAULT_BSE);
    }
}

void verifySensorAgreement() {
    // should notify telemetry if brakes have a disagreement
    if (abs(data.bseData.bseReading1 - data.bseData.bseReading2) > BSE_IMPLAUSABILITY_THRESHOLD) {
        // set fault code
    } else {
        // clear fault code
    }
}

void BSE_UpdateData(BSEData* bseData) {
    data.bseData.bseReading1 = bseData->bseReading1;
    data.bseData.bseReading2 = bseData->bseReading2;

    checkAndHandleBSEFault();
    verifySensorAgreement();
}

float BSE_GetBSEReading() {
    // regen braking
    return (data.bseData.bseReading1 + data.bseData.bseReading2) / 2;
}