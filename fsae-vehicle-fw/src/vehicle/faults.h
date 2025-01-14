// Anteater Electric Racing, 2025

#ifndef FAULTS_H
#define FAULTS_H

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
void Faults_Set(FaultType fault);
void Faults_Clear(FaultType fault);

#endif
