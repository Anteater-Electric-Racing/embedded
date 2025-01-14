// Anteater Electric Racing, 2025

#include "vehicle/faults.h"

typedef struct{
    bool overCurrent: 1;
    bool underVoltage: 1;
    bool overTemp: 1;
    bool APPS_fault: 1;
    bool BSE_fault: 1;
    bool APPS_brake_plausibility: 1;
} FaultMap;

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
