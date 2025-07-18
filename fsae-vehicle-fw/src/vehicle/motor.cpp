// Anteater Electric Racing, 2025


#define SPEED_CONTROL_ENABLED 0
#define SPEED_P_GAIN 0.01F // Proportional gain for speed control
#define SPEED_I_GAIN 0.1F // Integral gain for speed control

#include <arduino_freertos.h>

#include "utils/utils.h"

#include "peripherals/can.h"

#include "vehicle/motor.h"
#include "vehicle/telemetry.h"
#include "vehicle/rtm_button.h"
#include "apps.h"
#include "vehicle/ifl100-36.h"

typedef struct{
    MotorState state;
    float desiredTorque; // Torque demand in Nm;
} MotorData;

static MotorData motorData;
static TickType_t xLastWakeTime;
static VCU1 vcu1 = {0};
static BMS1 bms1 = {0};
static BMS2 bms2 = {0};

void Motor_Init(){
    motorData.state = MOTOR_STATE_OFF; // TODO Check if we want this
    motorData.desiredTorque = 0.0F; // No torque demand at start
}

void threadMotor(void *pvParameters){
    while(true){
        // Clear packet contents
        vcu1 = {0};
        bms1 = {0};
        bms2 = {0};
        switch (motorData.state){
            case MOTOR_STATE_OFF:
            {
                break;
            }
            case MOTOR_STATE_PRECHARGING:
            {
                // T2 State transition: BMS_Main_Relay_Cmd == 1 && Pre_charge_Relay_FB == 1
                vcu1.BMS_Main_Relay_Cmd = 1; // 1 = ON, 0 = OFF
                bms1.Pre_charge_Relay_FB = 1; // 1 = ON, 0 = OFF
                break;
            }
            case MOTOR_STATE_IDLE:
            {
                // T4 BMS_Main_Relay_Cmd == 1 && Pre_charge_Finish_Sts == 1 && Ubat>=200V
                vcu1.BMS_Main_Relay_Cmd = 1; // 1 = ON, 0 = OFF
                bms1.Pre_charge_Relay_FB = 1; // 1 = ON, 0 = OFF NOTE: see if we can omit this bit
                bms1.Pre_charge_Finish_Sts = 1; // 1 = ON, 0 = OFF
                break;
            }
            case MOTOR_STATE_DRIVING:
            {
                uint16_t maxDischarge = (uint16_t) (BATTERY_MAX_CURRENT_A + 500) * 10;
                uint16_t maxRegen = (uint16_t) (BATTERY_MAX_REGEN_A + 500) * 10;

                bms2.sAllowMaxDischarge = CHANGE_ENDIANESS_16(maxDischarge); // Convert to little-endian format
                bms2.sAllowMaxRegenCharge = CHANGE_ENDIANESS_16(maxRegen); // Convert to little-endian format

                // T5 BMS_Main_Relay_Cmd == 1 && VCU_MotorMode = 1/2
                vcu1.BMS_Main_Relay_Cmd = 1; // 1 = ON, 0 = OFF
                bms1.Pre_charge_Relay_FB = 1; // 1 = ON, 0 = OFF NOTE: see if we can omit this bit
                bms1.Pre_charge_Finish_Sts = 1; // 1 = ON, 0 = OFF

                vcu1.VehicleState = 1; // 0 = Not ready, 1 = Ready
                vcu1.GearLeverPos_Sts = 3; // 0 = Default, 1 = R, 2 = N, 3 = D, 4 = P
                vcu1.AC_Control_Cmd = 1; // 0 = Not active, 1 = Active
                vcu1.BMS_Aux_Relay_Cmd = 1; // 0 = not work, 1 = work
                vcu1.KeyPosition = 2; // 0 = Off, 1 = ACC, 2 = ON, 2 = Crank+On

                vcu1.VCU_TorqueReq = (uint8_t) ((fabsf(motorData.desiredTorque) / MOTOR_MAX_TORQUE) * 100); // Torque demand in percentage (0-99.6) 350Nm
                vcu1.VCU_MotorMode = motorData.desiredTorque >= 0 ? 1 : 2; // 0 = Standby, 1 = Drive, 2 = Generate Electricy, 3 = Reserved
                break;
            }
            case MOTOR_STATE_FAULT:
            {
                // T7 MCU_Warning_Level == 3
                vcu1.BMS_Main_Relay_Cmd = 1; // 1 = ON, 0 = OFF
                bms1.Pre_charge_Relay_FB = 1; // 1 = ON, 0 = OFF NOTE: see if we can omit this bit
                bms1.Pre_charge_Finish_Sts = 1; // 1 = ON, 0 = OFF
                vcu1.VCU_Warning_Level = 1; // 0 = No Warning, 1 = Warning, 2 = Fault, 3 = Critical Fault
                vcu1.VCU_MotorMode = 0; // 0 = Standby, 1 = Drive, 2 = Generate Electricy, 3 = Reserved
                break;
            }
            default:
            {
                break;
            }
        }

        vcu1.CheckSum = ComputeChecksum((uint8_t*) &vcu1);
        bms1.CheckSum = ComputeChecksum((uint8_t*) &bms1);
        bms2.CheckSum = ComputeChecksum((uint8_t*) &bms2);

        uint64_t vcu1_msg;
        memcpy(&vcu1_msg, &vcu1, sizeof(vcu1_msg));
        CAN_Send(mVCU1_ID, vcu1_msg);

        uint64_t bms1_msg;
        memcpy(&bms1_msg, &bms1, sizeof(bms1_msg));
        CAN_Send(mBMS1_ID, bms1_msg);

        uint64_t bms2_msg;
        memcpy(&bms2_msg, &bms2, sizeof(bms2_msg));
        CAN_Send(mBMS2_ID, bms2_msg);

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(20));
    }
}

void Motor_UpdateMotor(float torqueDemand, bool enablePrecharge, bool enablePower, bool enableRun, bool enableRegen){
    // Update the motor state based on the RTM button state

    // float throttleCommand = APPS_GetAPPSReading(); // 0; //TODO Get APPS_travel
    switch(motorData.state){
        // LV on, HV off
        case MOTOR_STATE_OFF:{
            if (enablePrecharge){
                # if DEBUG_FLAG
                    Serial.println("Precharging...");
                #endif
                motorData.state = MOTOR_STATE_PRECHARGING;
            }
            motorData.desiredTorque = 0.0F;
            break;
        }
        // HV switch on (PCC CAN message)
        case MOTOR_STATE_PRECHARGING:
        {
            if(enablePower){
                # if DEBUG_FLAG
                    Serial.println("Precharge finished");
                # endif
                motorData.state = MOTOR_STATE_IDLE;
            }
            motorData.desiredTorque = 0.0F;
            break;
        }
        // PCC CAN message finished
        case MOTOR_STATE_IDLE:
        {
            if(enableRun){
                # if DEBUG_FLAG
                    Serial.println("Ready to drive...");
                # endif
                motorData.state = MOTOR_STATE_DRIVING;
            }
            motorData.desiredTorque = 0.0F;
            break;
        }
        // Ready to drive button pressed
        case MOTOR_STATE_DRIVING:
        {
            // if(!enableRun){
            //     motorData.state = MOTOR_STATE_IDLE;
            // }

            // torque is communicated as a percentage
            #if !SPEED_CONTROL_ENABLED

            if (enableRegen && torqueDemand <= 0.0F && MCU_GetMCU1Data()->motorDirection == MOTOR_DIRECTION_FORWARD) {
                // If regen is enabled and the torque demand is zero, we need to set the torque demand to 0
                // to prevent the motor from applying torque in the wrong direction
                motorData.desiredTorque = MAX_REGEN_TORQUE * REGEN_BIAS;
            } else {
                motorData.desiredTorque = torqueDemand;
            }
            #else
            // Speed control is enabled, we need to set the torque demand to 0
            vcu1.VCU_TorqueReq = 0; // 0 = No torque
            #endif

            break;
        }
        case MOTOR_STATE_FAULT:
        {
            // TODO Implement RTM Button
            if(RTMButton_GetState() == false){
                motorData.state = MOTOR_STATE_IDLE;
            }
            motorData.desiredTorque = 0.0F;
            break;
        }
        default:
        {
            break;
        }
    }
}

float Motor_GetTorqueDemand(){
    return motorData.desiredTorque;
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

