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

const float IDLE_TS_VOLTAGE = 10.0F;
const float PRECH_ACCUM_VOLTAGE;
const float PRECH_ACCUM_TEMP;
const float PRECH_TS_VOLTAGE;

void stateMachineTask(void *pvParameters);

void Motor_Init();
void Motor_UpdateMotor();

bool Motor_TransitionToPrecharging();
bool Motor_TransitionToIdle();
bool Motor_TransitionToDriving();
//bool Motor_CheckReadyToDrive();


float Motor_DirectTorqueControl(float const &maxTorque);
float Motor_TorqueTractionControl(float &target_value, float &current_value, float &dt);
float Motor_GetTorqueDemand(); // implemented under the PID controller
float Motor_SetRearMotorTorque(float regenTorque);

void Motor_SetFaultState();

#endif
