// Anteater Electric Racing, 2025

#define THREAD_MOTOR_STACK_SIZE 128
#define THREAD_MOTOR_PRIORITY 1

#include <arduino_freertos.h>

#include "utils/utils.h"

#include "peripherals/can.h"

#include "vehicle/motor.h"
#include "vehicle/telemetry.h"
#include "vehicle/rtm_button.h"
#include "apps.h"

typedef struct{
    MotorState state;
    float torqueDemand;
} MotorData;

static MotorData motorData;
void threadMotor(void *pvParameters);

void Motor_Init(){
    motorData.state = MOTOR_STATE_OFF; // TODO Check if we want this
    xTaskCreate(threadMotor, "threadMotor", THREAD_MOTOR_STACK_SIZE, NULL, THREAD_MOTOR_PRIORITY, NULL);
}

void threadMotor(void *pvParameters){
    while(true){
        // Send CAN message to inverter
        CAN_SendVCU1Message(motorData.torqueDemand);
        vTaskDelay(10);
        # if DEBUG_FLAG
            Serial.print("Motor state is ");
            Serial.println(motorData.state);
            Serial.print("Motor torqueDemand is ");
            Serial.println(motorData.torqueDemand);
        # endif
    }
}

void Motor_UpdateMotor(){
    float throttleCommand = APPS_GetAPPSReading(); // 0; //TODO Get APPS_travel
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

