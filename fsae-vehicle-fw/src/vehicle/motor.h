// Anteater Electric Racing, 2025

#pragma once

#include <stdint.h>

typedef enum {
    MOTOR_STATE_OFF,
    MOTOR_STATE_PRECHARGING,
    MOTOR_STATE_IDLE,
    MOTOR_STATE_DRIVING,
    MOTOR_STATE_FAULT,
} MotorState;

const float MAX_SLIP_RATIO = 0.2;

void threadMotor(void *pvParameters);

void Motor_Init();
void Motor_UpdateMotor(float torqueDemand, bool enablePrecharge, bool enablePower, bool enableRun, bool enableRegen);

float Motor_GetTorqueDemand();
MotorState Motor_GetState();
void threadMotor(void *pvParameters);

void Motor_SetFaultState();
void Motor_ClearFaultState();
float TractionControl_GetSlip(float FL_speed, float FR_speed, float motorVelocity);
