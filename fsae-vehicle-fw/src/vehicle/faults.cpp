// Anteater Electric Racing, 2025

#define FAULT_OVER_CURRENT_MASK (0x1)
#define FAULT_UNDER_VOLTAGE_MASK (0x1 << 1)
#define FAULT_OVER_TEMP_MASK (0x1 << 2)
#define FAULT_APPS_MASK (0x1 << 3)
#define FAULT_BSE_MASK (0x1 << 4)
#define FAULT_BPPS_MASK (0x1 << 5)
#define FAULT_APPS_BRAKE_PLAUSIBILITY_MASK (0x1 << 6)

#include "vehicle/faults.h"
#include "vehicle/motor.h"
#include <Arduino.h> // TODO remove this, this is just for serial printing during testing

static volatile uint32_t faultBitMap;

void Faults_Init() {
    faultBitMap = 0;
    }

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
            Serial.println("Setting APPS fault");
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
            Serial.println("Setting APPS Plausibility fault");
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
        Serial.println("Clearing APPS fault in clr fault");
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
        Serial.println("Clearing APPS_BRAKE_PLAUSIBILITY fault in clr fault");
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

    Serial.print("Fault bitmap: ");
    Serial.println(faultBitMap);
    if (faultBitMap == 0) {
        Serial.println("Clearing all faults in handle faults");
        Motor_ClearFaultState();
        return;
    }
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
uint32_t Faults_GetFaults() { return faultBitMap; }

bool Faults_CheckAllClear() { return faultBitMap == 0; }
