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
static VCU1 vcu1 = {0};
static BMS1 bms1 = {0};
static BMS2 bms2 = {0};
static void threadMotor(void *pvParameters);

void Motor_Init(){
    motorData.state = MOTOR_STATE_IDLE;
    Serial.println("Motor thread starting");
    xTaskCreate(threadMotor, "threadMotor", THREAD_MOTOR_STACK_SIZE, NULL, THREAD_MOTOR_PRIORITY, NULL);
}

static void threadMotor(void *pvParameters){
    while(true){

        switch (motorData.state){
            case MOTOR_STATE_OFF:
            case MOTOR_STATE_PRECHARGING:
            {
                // T2 BMS_Main_Relay_Cmd == 1 && Pre_charge_Relay_FB == 1
                vcu1.BMS_Main_Relay_Cmd = 1; // 1 = ON, 0 = OFF
                bms1.Pre_charge_Relay_FB = 1; // 1 = ON, 0 = OFF

                vcu1.VCU_TorqueReq = 0; // 0 = No torque
            }
            case MOTOR_STATE_IDLE:
            {
                // T4 BMS_Main_Relay_Cmd == 1 && Pre_charge_Finish_Sts == 1 && Ubat>=200V
                vcu1.BMS_Main_Relay_Cmd = 1; // 1 = ON, 0 = OFF
                bms1.Pre_charge_Finish_Sts = 1; // 1 = ON, 0 = OFF
                
                bms1.Fast_charge_Relay_FB = 1;
                bms2.sAllowMaxDischarge = 500;
                bms2.sAllowMaxRegenCharge = 500;

                vcu1.VCU_TorqueReq = 0; // 0 = No torque
                vcu1.VehicleState = 1; // 0 = Not ready, 1 = Ready
                vcu1.GearLeverPos_Sts = 3; 
                vcu1.AC_Control_Cmd = 1;
                vcu1.BMS_Aux_Relay_Cmd = 1;
                vcu1.VCU_MotorMode = 1;
                vcu1.KeyPosition = 2;
            }
            case MOTOR_STATE_DRIVING:
            {
                // T5 BMS_Main_Relay_Cmd == 1 && VCU_MotorMode = 1/2
                vcu1.BMS_Main_Relay_Cmd = 1; // 1 = ON, 0 = OFF
                vcu1.VCU_MotorMode = 1; // 0 = Standby, 1 = Drive, 2 = Generate Electricy, 3 = Reserved
                
                vcu1.VCU_TorqueReq = motorData.torqueDemand; // Troque demand in percentage (0-99.6)
            }
            case MOTOR_STATE_FAULT:
            {
                // T7 MCU_Warning_Level == 3
                vcu1.VCU_Warning_Level = 3; // 0 = No Warning, 1 = Warning, 2 = Fault, 3 = Critical Fault

                vcu1.VCU_TorqueReq = 0; // 0 = No torque
            }
            default:
                break;
        }

        uint64_t vcu1_msg;
        memcpy(&vcu1_msg, &vcu1, sizeof(vcu1_msg));
        CAN_Send(mVCU1_ID, vcu1_msg);

        uint64_t bms1_msg;
        memcpy(&bms1_msg, &bms1, sizeof(bms1_msg));
        CAN_Send(mBMS1_ID, bms1_msg);

        uint64_t bms2_msg;
        memcpy(&bms2_msg, &bms2, sizeof(bms2_msg));
        CAN_Send(mBMS1_ID, bms2_msg);

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(20));
    }
}

void Motor_UpdateMotor(){
    float throttleCommand = Telemetry_GetData()->APPS_Travel;
    switch(motorData.state){
        // LV on, HV off
        case MOTOR_STATE_OFF:
        // HV swtich on (PCC CAN message)
        case MOTOR_STATE_PRECHARGING:
        {
            if(RTMButton_GetState()){
                motorData.state = MOTOR_STATE_IDLE;
            }
            
            motorData.torqueDemand = 0.0F;
            break;
        }
        // PCC CAN message finished
        case MOTOR_STATE_IDLE:
        {
            if(RTMButton_GetState()){
                motorData.state = MOTOR_STATE_DRIVING;
            }
            break;
        }
        // Ready to drive button pressed
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

