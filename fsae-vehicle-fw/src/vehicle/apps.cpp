// Anteater Electric Racing, 2025

#define APPS_IMPLAUSABILITY_THRESHOLD 0.1

#include <cmath>

#include "apps.h"

#include "vehicle/faults.h"

APPS::APPS() {
    _APPSdata.appsReading1 = 0;
    _APPSdata.appsReading1 = 0;
    APPS_fault = false;
}

APPS::~APPS() {}

void APPS::checkAPPSFault() {
    if (abs(_APPSdata.appsReading1 - _APPSdata.appsReading2) > APPS_IMPLAUSABILITY_THRESHOLD) {
        APPS_fault = true;
        Faults_SetFault(FAULT_APPS);
        // motor_setFaultState();
    }
}

void APPS::clearAPPSFault() {
    if (abs(_APPSdata.appsReading1 - _APPSdata.appsReading2) < APPS_IMPLAUSABILITY_THRESHOLD){
        APPS_fault = false;
        Faults_ClearFault(FAULT_APPS);
        // send command to another module that checks if all faults are clear - fault.sendAllClear();
    }
}

void APPS::APPS_UpdateData(APPSData* data) {
    _APPSdata.appsReading1 = data->appsReading1;
    _APPSdata.appsReading2 = data->appsReading2;

    if (!APPS_fault) {
        // If not already faulted, check for fault
        checkAPPSFault();
    } else {
        // If already faulted, check if fault can be cleared
        clearAPPSFault();
    }

    // send to telemetry (or should this be in the adc loop?)
    // but need to send to telemetry that there is a apps fault
}
