// Anteater Electric Racing, 2025

#include "vehicle/faults.h"
#include "vehicle/motor.h"

FaultMap faultMap;

void Faults_Init(){
    faultMap = {
        .overCurrent = false,
        .underVoltage = false,
        .overTemp = false,
        .APPS_fault = false,
        .BSE_fault = false,
        .APPS_brake_plausibility = false
    };
}

void Faults_SetFault(FaultType fault){
    switch(fault){
        case FAULT_NONE:
        {
            break;
        }
        case FAULT_OVER_CURRENT:
        {
            faultMap.overCurrent = true;
            break;
        }
        case FAULT_UNDER_VOLTAGE:
        {
            faultMap.underVoltage = true;
            break;
        }
        case FAULT_OVER_TEMP:
        {
            faultMap.overTemp = true;
            break;
        }
        case FAULT_APPS:
        {
            faultMap.APPS_fault = true;
            break;
        }
        case FAULT_BSE:
        {
            faultMap.BSE_fault = true;
            break;
        }
        case FAULT_APPS_BRAKE_PLAUSIBILITY:
        {
            faultMap.APPS_brake_plausibility = true;
            break;
        }
        default:
        {
            break;
        }
    }
}

// for telemetry
FaultMap* Faults_GetFaults() {
    return &faultMap;
}

void Faults_ClearFault(FaultType fault){
    switch(fault){
        case FAULT_NONE:
        {
            break;
        }
        case FAULT_OVER_CURRENT:
        {
            faultMap.overCurrent = false;
            break;
        }
        case FAULT_UNDER_VOLTAGE:
        {
            faultMap.underVoltage = false;
            break;
        }
        case FAULT_OVER_TEMP:
        {
            faultMap.overTemp = false;
            break;
        }
        case FAULT_APPS:
        {
            faultMap.APPS_fault = false;
            break;
        }
        case FAULT_BSE:
        {
            faultMap.BSE_fault = false;
            break;
        }
        case FAULT_APPS_BRAKE_PLAUSIBILITY:
        {
            faultMap.APPS_brake_plausibility = false;
            break;
        }
        default:
        {
            break;
        }
    }
}

// need struct of car info
// fault logging?
void Faults_CheckFaults() {
    if (checkOverCurrent())
        Faults_SetFault(FAULT_OVER_CURRENT);
    if (checkUnderVoltage())
        Faults_SetFault(FAULT_UNDER_VOLTAGE);
    if (checkOverTemp())
        Faults_SetFault(FAULT_OVER_TEMP);
    if (checkAPPS())
        Faults_SetFault(FAULT_APPS);
    if (checkBSE())
        Faults_SetFault(FAULT_BSE);
    if (checkABP())
        Faults_SetFault(FAULT_APPS_BRAKE_PLAUSIBILITY);
}


bool checkOverCurrent() {
    // battery shutdown
    return false;
}

bool checkUnderVoltage() {
    // battery shutdown
    return false;
}

bool checkOverTemp() {
    // battery shutdown
    return false;
}

bool checkAPPS() {
    // APPS check for 10% difference in throttle inputs
    // Motor_SetFaultState();
    return false;
}

bool checkBSE() {
    // BSE check for signal in range of 0.5-4.5 V
    // Motor_SetFaultState();
    return false;
}

bool checkABP() {
    // ABP check for brakes engaged + APPS more than 25% throttle
    // Motor_SetFaultState();
    return false;
}

void Faults_HandleFaults() {
    if (faultMap.overCurrent) {
        // battery on
        Faults_ClearFault(FAULT_OVER_CURRENT);
    }
    if (faultMap.underVoltage) {
        // battery on
        Faults_ClearFault(FAULT_UNDER_VOLTAGE);
    }
    if (faultMap.overTemp) {
        // battery on
        Faults_ClearFault(FAULT_OVER_TEMP);
    }
    if (faultMap.APPS_fault) {
        // clear fault and motor on when throttle is within 10% difference
        Faults_ClearFault(FAULT_APPS);
    }
    if (faultMap.BSE_fault) {
        // clear fault and motor on when brake signal is within range 0.5-4.5 V
        Faults_ClearFault(FAULT_BSE);
    }
    if (faultMap.APPS_brake_plausibility) {
        // clear fault and motor on when throttle is less than 5% travel
        Faults_ClearFault(FAULT_APPS_BRAKE_PLAUSIBILITY);
    }

    // final check before reset
    if (Faults_CheckAllClear()) {
        // check if motor/battery faulted
            // set motor state to driving
            // reset battery
    }
}

bool Faults_CheckAllClear() {
    return !faultMap.overCurrent && !faultMap.underVoltage && !faultMap.overTemp && !faultMap.APPS_fault && !faultMap.BSE_fault && !faultMap.APPS_brake_plausibility;
}