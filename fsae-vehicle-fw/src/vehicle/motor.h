// Anteater Electric Racing, 2025

#pragma once

#include "ifl100-36.h"

#include <stdint.h>

typedef enum {
    MOTOR_STATE_OFF,
    MOTOR_STATE_STANDBY,
    MOTOR_STATE_PRECHARGING,
    MOTOR_STATE_IDLE,
    MOTOR_STATE_DRIVING,
    MOTOR_STATE_FAULT
} MotorState;

void threadMotor(void *pvParameters);

void Motor_Init();
void Motor_UpdateMotor(float torqueDemand);
void Motor_UpdateMotor(float torqueDemand, bool enablePrecharge,
                       bool enablePower, bool enableRun, bool enableRegen,
                       bool enableStandby);
float torqueMap(float pedal);
float Motor_GetTorqueDemand();
MotorState Motor_GetState();
void threadMotor(void *pvParameters);

void Motor_SetFaultState();
void Motor_ClearFaultState();
void Motor_ClearToIdleFault();
float Motor_TargetTorque();
float Motor_desierdTorque();

VCU1 *VCU_GetRawVCU();
