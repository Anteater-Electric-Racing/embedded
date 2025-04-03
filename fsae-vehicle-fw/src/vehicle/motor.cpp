// Anteater Electric Racing, 2025

#include "vehicle/motor.h"
#include "vehicle/telemetry.h"

typedef struct{
    MotorState state;
    float torqueDemand;
} MotorData;

MotorData motorData;

void Motor_Init(){
    motorData.state = MOTOR_STATE_OFF;
}

void Motor_UpdateMotor(){
    switch(motorData.state){
        case MOTOR_STATE_OFF:
        {
            break;
        }
        case MOTOR_STATE_PRECHARGING:
        {
            break;
        }
        case MOTOR_STATE_IDLE:
        {
            // check for ready to drive button press
            break;
        }
        case MOTOR_STATE_DRIVING:
        {
            // apps.getThrottle() for setpoint maybe?
            // pid.compute() for torque demand
            motorData.torqueDemand = 69.69F;
            break;
        }
        case MOTOR_STATE_FAULT:
        {
            motorData.torqueDemand = 0.0F;
            break;
        }
        default:
        {
            break;
        }
    }
}

float Motor_GetTorqueDemand(){
    return motorData.torqueDemand;
}

void Motor_SetFaultState(){
    motorData.state = MOTOR_STATE_FAULT;
}

