// Anteater Electric Racing, 2025

#ifndef FAULTS_H
#define FAULTS_H

#define FAULT_OVER_CURRENT_OFFSET 0x1
#define FAULT_UNDER_VOLTAGE_OFFSET 0x1 << 1
#define FAULT_OVER_TEMP_OFFSET 0x1 << 2
#define FAULT_APPS_OFFSET 0x1 << 3
#define FAULT_BSE_OFFSET 0x1 << 4
#define FAULT_APPS_BRAKE_PLAUSIBILITY_OFFSET 0x1 << 5

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
void Faults_CheckFaults();
void Faults_HandleFaults();
bool Faults_CheckAllClear();

bool checkOverCurrent();
bool checkUnderVoltage();
bool checkOverTemp();
bool checkAPPS();
bool checkBSE();
bool checkABP();



#endif
