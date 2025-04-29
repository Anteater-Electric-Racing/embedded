// Anteater Electric Racing, 2025

#include "faults.h"
#include "motor.h"

static uint32_t faultBitMap;

void Faults_Init() { faultBitMap = 0; }

void Faults_SetFault(FaultType fault) {
    switch (fault) {
        case FAULT_NONE: {
            break;
        }
        case FAULT_OVER_CURRENT: {
            faultBitMap |= FAULT_OVER_CURRENT_MASK;
            break;
        }
        case FAULT_UNDER_VOLTAGE: {
            faultBitMap |= FAULT_UNDER_VOLTAGE_MASK;
            break;
        }
        case FAULT_OVER_TEMP: {
            faultBitMap |= FAULT_OVER_TEMP_MASK;
            break;
        }
        case FAULT_APPS: {
            faultBitMap |= FAULT_APPS_MASK;
            break;
        }
        case FAULT_BSE: {
            faultBitMap |= FAULT_BSE_MASK;
            break;
        }
        case FAULT_BPPS: {
            faultBitMap |= FAULT_BPPS_MASK;
            break;
        }
        case FAULT_APPS_BRAKE_PLAUSIBILITY: {
            faultBitMap |= FAULT_APPS_BRAKE_PLAUSIBILITY_MASK;
            break;
        }
        default: {
            break;
        }
    }
}

void Faults_ClearFault(FaultType fault) {
    switch (fault) {
        case FAULT_NONE: {
            break;
        }
        case FAULT_OVER_CURRENT: {
            faultBitMap &= ~FAULT_OVER_CURRENT_MASK;
            break;
        }
        case FAULT_UNDER_VOLTAGE: {
            faultBitMap &= ~FAULT_UNDER_VOLTAGE_MASK;
            break;
        }
        case FAULT_OVER_TEMP: {
            faultBitMap &= ~FAULT_OVER_TEMP_MASK;
            break;
        }
        case FAULT_APPS: {
            faultBitMap &= ~FAULT_APPS_MASK;
            break;
        }
        case FAULT_BSE: {
            faultBitMap &= ~FAULT_BSE_MASK;
            break;
        }
        case FAULT_BPPS: {
            faultBitMap &= ~FAULT_BPPS_MASK;
            break;
        }
        case FAULT_APPS_BRAKE_PLAUSIBILITY: {
            faultBitMap &= ~FAULT_APPS_BRAKE_PLAUSIBILITY_MASK;
            break;
        }
        default: {
            break;
        }
    }
}

// currently having all faults being handled the same but leaving room for
// future customization
void Faults_HandleFaults() {
    if (faultBitMap & FAULT_OVER_CURRENT_MASK) {
        Motor_SetFaultState();
    }
    if (faultBitMap & FAULT_UNDER_VOLTAGE_MASK) {
        Motor_SetFaultState();
    }
    if (faultBitMap & FAULT_OVER_TEMP_MASK) {
        Motor_SetFaultState();
    }
    if (faultBitMap & FAULT_APPS_MASK) {
        Motor_SetFaultState();
    }
    if (faultBitMap & FAULT_BSE_MASK) {
        Motor_SetFaultState();
    }
    if (faultBitMap & FAULT_BPPS_MASK) {
        Motor_SetFaultState();
    }
    if (faultBitMap & FAULT_APPS_BRAKE_PLAUSIBILITY_MASK) {
        Motor_SetFaultState();
    }
}

// for telemetry
uint32_t *Faults_GetFaults() { return &faultBitMap; }

bool Faults_CheckAllClear() { return faultBitMap == 0; }
