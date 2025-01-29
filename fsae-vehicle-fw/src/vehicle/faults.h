// Anteater Electric Racing, 2025

#ifndef FAULTS_H
#define FAULTS_H

#include <stdint.h>

typedef enum {
    FAULT_NONE,
    FAULT_OVER_CURRENT,
    FAULT_UNDER_VOLTAGE,
    FAULT_OVER_TEMP,
    FAULT_APPS,
    FAULT_BSE,
    FAULT_APPS_BRAKE_PLAUSIBILITY
} FaultType;


void Faults_Init();

void Faults_SetFault(FaultType fault);
uint32_t* Faults_GetFaults();
void Faults_ClearFault(FaultType fault);
void Faults_HandleFaults();
bool Faults_CheckAllClear();

#endif
