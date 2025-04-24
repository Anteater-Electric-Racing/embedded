// Anteater Electric Racing, 2025

#include "utils/utils.h"

#include "vehicle/motor.h"
#include "vehicle/telemetry.h"
#include "vehicle/rtm_button.h"

typedef struct{
    MotorState state;
    float torqueDemand;
} MotorData;

MotorData motorData;

void Motor_Init(){
    motorData.state = MOTOR_STATE_IDLE;
}

void Motor_UpdateMotor(){
    float throttleCommand = Telemetry_GetData()->APPS_Travel;
    switch(motorData.state){
        case MOTOR_STATE_OFF:
        case MOTOR_STATE_PRECHARGING:
        case MOTOR_STATE_IDLE:
        {
            if(RTMButton_GetState()){
                motorData.state = MOTOR_STATE_DRIVING;
            }
            break;
        }
        case MOTOR_STATE_DRIVING:
        {
            if(RTMButton_GetState() == false){
                motorData.state = MOTOR_STATE_IDLE;
            }

            // torque is communicated as a percentage
            motorData.torqueDemand = throttleCommand;
            break;
        }
        case MOTOR_STATE_FAULT:
        {
            if(RTMButton_GetState() == false){
                motorData.state = MOTOR_STATE_IDLE;
            }
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

