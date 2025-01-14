// Anteater Electric Racing, 2025

#include "vehicle/faults.h"

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

void Faults_Set(FaultType fault){
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
FaultMap Faults_Get() {
    return faultMap;
}

void Faults_Clear(FaultType fault){
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
void Faults_Check() {
    if (checkOverCurrent())
        Faults_Set(FAULT_OVER_CURRENT);
    if (checkUnderVoltage())
        Faults_Set(FAULT_UNDER_VOLTAGE);
    if (checkOverTemp())
        Faults_Set(FAULT_OVER_TEMP);
    if (checkAPPS())
        Faults_Set(FAULT_APPS);
    if (checkBSE())
        Faults_Set(FAULT_BSE);
    if (checkABP())
        Faults_Set(FAULT_APPS_BRAKE_PLAUSIBILITY);
}


bool checkOverCurrent() {
    return false;
}

bool checkUnderVoltage() {
    return false;
}

bool checkOverTemp() {
    return false;
}

bool checkAPPS() {
    // APPS check for 10% difference in throttle inputs
    // interface with module to turn off TS (power to motors shut off)
    // handle APPS ? 
    return false;
}

bool checkBSE() {
    // BSE check for signal in range of 0.5-4.5 V
    // interface with module to turn off TS (power to motors shut off)
    // handle BSE ? 
    return false;
}

bool checkABP() {
    // ABP check for brakes engaged + APPS more than 25%
    // interface with module to turn off TS (power to motors shut off)
    // handle ABP ? 
    return false;
}

void Faults_Handle() {
    if (faultMap.overCurrent) {
        // handle
        Faults_Clear(FAULT_OVER_CURRENT);
    }
}