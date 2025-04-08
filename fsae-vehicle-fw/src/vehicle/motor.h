// Anteater Electric Racing, 2025

#ifndef MOTOR_H
#define MOTOR_H

typedef enum {
    MOTOR_STATE_OFF,
    MOTOR_STATE_PRECHARGING,
    MOTOR_STATE_IDLE,
    MOTOR_STATE_DRIVING,
    MOTOR_STATE_FAULT,
} MotorState;

void Motor_Init();
void Motor_UpdateMotor();

float Motor_GetTorqueDemand();

void Motor_SetFaultState();

#endif
