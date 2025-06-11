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

void threadMotor(void *pvParameters);

void Motor_Init();
void Motor_UpdateMotor();

float Motor_GetTorqueDemand();
MotorState Motor_GetState();
void threadMotor(void *pvParameters);

void Motor_SetFaultState();
void Motor_ClearFaultState();

void Motor_UpdatePrechargeState(uint64_t* rx_data);
