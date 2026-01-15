// Anteater Electric Racing, 2025

#pragma once

#include <stdint.h>

typedef enum {
    FAULT_NONE,
    FAULT_OVER_CURRENT,
    FAULT_UNDER_VOLTAGE,
    FAULT_OVER_TEMP,
    FAULT_APPS,
    FAULT_BSE,
    FAULT_BPPS,
    FAULT_APPS_BRAKE_PLAUSIBILITY,

    // Inverter Faults
    FAULT_DC_OVER_VOLT,
    FAULT_MOTOR_PHASE_CURR,
    FAULT_OVER_HOT,
    FAULT_RESOLVER,
    FAULT_PHASE_CURR_SENSOR,
    FAULT_MOTOR_OVER_SPEED,
    FAULT_DRV_OVER_HOT,
    FAULT_DC_OVER_CURR,
    FAULT_DRV_OVER_COOL,
    FAULT_DC_LOW_VOLT_WARN,
    FAULT_12V_LOW_VOLT_WARN,
    FAULT_MOTOR_STALL,
    FAULT_MOTOR_OPEN_PHASE,


} FaultType;



void Faults_Init();

void Faults_SetFault(FaultType fault);
uint32_t Faults_GetFaults();
void Faults_ClearFault(FaultType fault);
void Faults_HandleFaults();
bool Faults_CheckAllClear();
