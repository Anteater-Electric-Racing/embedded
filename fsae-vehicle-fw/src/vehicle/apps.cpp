// Anteater Electric Racing, 2025

#define APPS_IMPLAUSABILITY_THRESHOLD 0.1
#define APPS_BSE_PLAUSABILITY_TROTTLE_THRESHOLD 0.25
#define APPS_BSE_PLAUSABILITY_BRAKE_THRESHOLD 0.1 // ???

#define BSE_LOWER_THRESHOLD 0.5
#define BSE_UPPER_THRESHOLD 4.5
#define BSE_IMPLAUSABILITY_THRESHOLD 0.1

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
        // is it ok to always call clear fault
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

BSE::BSE() {
    _BSEdata.BSEReading1 = 0;
    _BSEdata.BSEReading2 = 0;
}

BSE::~BSE() {}

void BSE::checkAndHandleBSEFault() {
    if (_BSEdata.BSEReading1 < BSE_LOWER_THRESHOLD ||
        _BSEdata.BSEReading1 > BSE_UPPER_THRESHOLD) {
        Faults_SetFault(FAULT_BSE);
    } else {
        Faults_ClearFault(FAULT_BSE);
    }
}

void BSE::verifySensorAgreement() {
    // should notify telemetry if brakes have a disagreement
    if (abs(_BSEdata.BSEReading1 - _BSEdata.BSEReading2) >
        BSE_IMPLAUSABILITY_THRESHOLD) {
        // set fault code
    } else {
        // clear fault code
    }
}

void BSE::BSE_UpdateData(APPSData *data) {
    _BSEdata.BSEReading1 = data->appsReading1;
    _BSEdata.BSEReading2 = data->appsReading2;

    checkAndHandleBSEFault();
    verifySensorAgreement();
}

float BSE::BSE_GetBSEReading() {
    // regen braking
    return (_BSEdata.BSEReading1 + _BSEdata.BSEReading2) / 2;
}
