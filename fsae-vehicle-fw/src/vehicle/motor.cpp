// Anteater Electric Racing, 2025

#define THREAD_MOTOR_STACK_SIZE 128
#define THREAD_MOTOR_PRIORITY 1

#include <arduino_freertos.h>

#include "utils/utils.h"

#include "peripherals/can.h"

#include "vehicle/motor.h"
#include "vehicle/telemetry.h"
#include "vehicle/rtm_button.h"
#include "vehicle/ifl100-36.h"

typedef struct{
    MotorState state;
    float torqueDemand;
} MotorData;

static MotorData motorData;
static TickType_t xLastWakeTime;
void threadMotor(void *pvParameters);

void Motor_Init(){
    motorData.state = MOTOR_STATE_IDLE;
    xTaskCreate(threadMotor, "threadMotor", THREAD_MOTOR_STACK_SIZE, NULL, THREAD_MOTOR_PRIORITY, NULL);
}

void threadMotor(void *pvParameters){
    while(true){
        // Send CAN message to inverter
        Serial.println("Motor thread running");
        
        VCU1 vcu1 = {0};
        vcu1.sMainRelayCmd = 1; // 1 = ON, 0 = OFF
        uint64_t vcu1_msg;
        memcpy(&vcu1_msg, &vcu1, sizeof(vcu1_msg));
        CAN_SendVCU1Message(mVCU1_ID, vcu1_msg);
        
        BMS1 bms1 = {0};
        bms1.sPrechargeRelay_FB = 1; // 1 = ON, 0 = OFF
        // Print out the bytes of bms1
        uint8_t* bms1_bytes = reinterpret_cast<uint8_t*>(&bms1);
        for (size_t i = 0; i < sizeof(bms1); ++i) {
            Serial.print("bms1 byte[");
            Serial.print(i);
            Serial.print("]: ");
            Serial.println(bms1_bytes[i], 16);
        }
        uint64_t bms1_msg;
        memcpy(&bms1_msg, &bms1, sizeof(bms1_msg));
        CAN_SendVCU1Message(mBMS1_ID, bms1_msg);
        // vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(20));
    }
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

