// Anteater Electric Racing, 2025

#include "utils/utils.h"

#include "vehicle/motor.h"
#include "vehicle/telemetry.h"

typedef struct{
    MotorState state;
    float torqueDemand;
} MotorData;

MotorData motorData;

void Motor_Init(){
    motorData.state = MOTOR_STATE_DRIVING;
}

void Motor_UpdateMotor(){
    float throttleCommand = Telemetry_GetData()->APPS_Travel;
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
            motorData.torqueDemand = MOTOR_MAX_TORQUE * throttleCommand;
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

void Motor_ClearFaultState(){
    motorData.state = MOTOR_STATE_DRIVING;
}

MotorState Motor_GetState(){
    return motorData.state;
}

