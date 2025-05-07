// Anteater Electric Racing, 2025

#pragma once

#include <stdint.h>

#define FAULT_OVER_CURRENT_MASK 0x1
#define FAULT_UNDER_VOLTAGE_MASK (0x1 << 1)
#define FAULT_OVER_TEMP_MASK (0x1 << 2)
#define FAULT_APPS_MASK (0x1 << 3)
#define FAULT_BSE_MASK (0x1 << 4)
#define FAULT_BPPS_MASK (0x1 << 5)
#define FAULT_APPS_BRAKE_PLAUSIBILITY_MASK (0x1 << 6)

typedef enum {
    FAULT_NONE,
    FAULT_OVER_CURRENT,
    FAULT_UNDER_VOLTAGE,
    FAULT_OVER_TEMP,
    FAULT_APPS,
    FAULT_BSE,
    FAULT_BPPS,
    FAULT_APPS_BRAKE_PLAUSIBILITY
} FaultType;

void Faults_Init();

void Faults_SetFault(FaultType fault);
uint32_t *Faults_GetFaults();
void Faults_ClearFault(FaultType fault);
void Faults_HandleFaults();
bool Faults_CheckAllClear();
