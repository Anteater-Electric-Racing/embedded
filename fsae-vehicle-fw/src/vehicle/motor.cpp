// Anteater Electric Racing, 2025

#define SPEED_CONTROL_ENABLED 0
#define SPEED_P_GAIN 0.01F // Proportional gain for speed control
#define SPEED_I_GAIN 0.1F  // Integral gain for speed control

#include <arduino_freertos.h>
#include "utils/utils.h"
#include "peripherals/can.h"
#include "peripherals/gpio.h"

#include "vehicle/apps.h"
#include "vehicle/bse.h"
#include "vehicle/faults.h"
#include "vehicle/ifl100-36.h"
#include "vehicle/motor.h"
#include "vehicle/pcc_receive.h"
#include "vehicle/rtm_button.h"
#include "vehicle/telemetry.h"

typedef struct {
    MotorState state;
    float desiredTorque; // Torque demand in Nm;
} MotorData;

static MotorData motorData;
static TickType_t xLastWakeTime;
static VCU1 vcu1 = {0};
static BMS1 bms1 = {0};
static BMS2 bms2 = {0};

void Motor_Init() {
    motorData.state = MOTOR_STATE_PRECHARGING; // DEFAULT TO PRECHARGE
    motorData.desiredTorque = 0.0F;            // No torque demand at start
}

void threadMotor(void *pvParameters) {
    while (true) {
        // Clear packet contents
        vcu1 = {0};
        bms1 = {0};
        bms2 = {0};

        switch (motorData.state) {
        case MOTOR_STATE_OFF: {
            break;
        }

        case MOTOR_STATE_STANDBY: {

            vcu1.BMS_Main_Relay_Cmd = 0;
            bms1.Pre_charge_Relay_FB = 0; // 1 = ON, 0 = OFF
            bms1.Pre_charge_Finish_Sts = 0;
            break;
        }

        case MOTOR_STATE_PRECHARGING: {
            vcu1.BMS_Main_Relay_Cmd = 1;  // 1 = ON, 0 = OFF
            bms1.Pre_charge_Relay_FB = 1; // 1 = ON, 0 = OFF
            vcu1.VCU_TorqueReq = 0;
            break;
        }

        case MOTOR_STATE_IDLE: {

            vcu1.BMS_Main_Relay_Cmd = 1;    // 1 = ON, 0 = OFF
            bms1.Pre_charge_Relay_FB = 1;   // 1 = ON, 0 = OFF
            bms1.Pre_charge_Finish_Sts = 1; // 1 = ON, 0 = OFF

            // T6
            vcu1.VCU_MotorMode = 0;
            vcu1.VCU_TorqueReq = 0;
            break;
        }

        case MOTOR_STATE_DRIVING: {

            /*============VARIABLE POWER DERATING (PREREQ: BMS) ============*/
            // float maxBatteryCurrent = min(BATTERY_MAX_CURRENT_A, INFINITY);
            // float maxRegenCurrent = min(BATTERY_MAX_REGEN_A, INFINITY);

            // pass into maxDischarge/maxRegen fields

            uint16_t maxDischarge =
                (uint16_t)(BATTERY_MAX_CURRENT_A + 500) * 10;
            uint16_t maxRegen = (uint16_t)(BATTERY_MAX_REGEN_A + 500) * 10;
            bms2.sAllowMaxDischarge = CHANGE_ENDIANESS_16(maxDischarge);
            bms2.sAllowMaxRegenCharge = CHANGE_ENDIANESS_16(
                maxRegen); // Convert to little-endian format

            // T5 BMS_Main_Relay_Cmd == 1 && VCU_MotorMode = 1/2
            vcu1.BMS_Main_Relay_Cmd = 1;    // 1 = ON, 0 = OFF
            bms1.Pre_charge_Relay_FB = 1;   // 1 = ON, 0 = OFF
            bms1.Pre_charge_Finish_Sts = 1; // 1 = ON, 0 = OFF

            vcu1.VehicleState = 1; // 0 = Not ready, 1 = Ready

            /* Switched to reverse */
            vcu1.GearLeverPos_Sts =
                1;                   // 0 = Default, 1 = R, 2 = N, 3 = D, 4 = P
            vcu1.AC_Control_Cmd = 1; // 0 = Not active, 1 = Active
            vcu1.BMS_Aux_Relay_Cmd = 1; // 0 = not work, 1 = work
            vcu1.VCU_WorkMode = 0;
            vcu1.VCU_TorqueReq =
                (uint8_t)((fabsf(motorData.desiredTorque) / MOTOR_MAX_TORQUE) *
                          100); // Torque demand in percentage (0-99.6) 350Nm
            vcu1.VCU_MotorMode = motorData.desiredTorque >= 0
                                     ? 1
                                     : 2; // 0 = Standby, 1 = Drive, 2 =
                                          // Generate Electricy, 3 = Reserved
            break;
        }

        case MOTOR_STATE_FAULT: {
            // T7 MCU_Warning_Level == 3
            vcu1.BMS_Main_Relay_Cmd = 0;    // 1     = ON, 0 = OFF
            bms1.Pre_charge_Relay_FB = 0;   // 1     = ON, 0 = OFF
            bms1.Pre_charge_Finish_Sts = 0; // 1   = ON, 0 = OFF
            vcu1.VCU_MotorMode = 0; // 1       0 = Standby, 1 = Drive, 2 =
                                    // Generate// Electricy, 3 = Reserved
            break;
        }

        default: {
            break;
        }
        }

        vcu1.CheckSum = ComputeChecksum((uint8_t *)&vcu1);
        bms1.CheckSum = ComputeChecksum((uint8_t *)&bms1);
        bms2.CheckSum = ComputeChecksum((uint8_t *)&bms2);

        uint64_t vcu1_msg;
        memcpy(&vcu1_msg, &vcu1, sizeof(vcu1_msg));
        CAN_Send(mVCU1_ID, vcu1_msg);

        uint64_t bms1_msg;
        memcpy(&bms1_msg, &bms1, sizeof(bms1_msg));
        CAN_Send(mBMS1_ID, bms1_msg);

        uint64_t bms2_msg;
        memcpy(&bms2_msg, &bms2, sizeof(bms2_msg));
        CAN_Send(mBMS2_ID, bms2_msg);

        float pedalTorque;
        if (APPS_GetAPPSReading1() > 0.125) {
            pedalTorque = APPS_GetAPPSReading1() * (MOTOR_MAX_TORQUE);
        } else {
            pedalTorque = 0;
        }

#if !HIMAC_FLAG
        Motor_UpdateMotor(pedalTorque);
#endif

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(20));
    }
}

#if !HIMAC_FLAG
void Motor_UpdateMotor(float torqueDemand) {

    Faults_HandleFaults();
    RTMButton_Update(GPIO_Read(RTM_BUTTON_PIN));

    switch (motorData.state) {
    case MOTOR_STATE_OFF:
        motorData.desiredTorque = 0.0F;
        break;
    case MOTOR_STATE_STANDBY:
        motorData.desiredTorque = 0.0F;
        break;
    case MOTOR_STATE_PRECHARGING: /* default state */
        if ((PCC_GetData()->prechargeProgress >= 85 &&
             PCC_GetData()->state == 3)) {
            motorData.state = MOTOR_STATE_IDLE;
        }
        motorData.desiredTorque = 0.0F;
        break;
    case MOTOR_STATE_IDLE:
        if (RTMButton_GetState() &&
            Faults_CheckAllClear()) { // transition to IDLE
            // TODO Update brake light threshold if we only want to move when mech brakes are engaged
            if (BSE_GetBSEReading()->bseFront_Reading >= BRAKE_LIGHT_THRESHOLD &&
                BSE_GetBSEReading()->bseRear_Reading >= BRAKE_LIGHT_THRESHOLD){
                    motorData.state = MOTOR_STATE_DRIVING;
            }
        }
        motorData.desiredTorque = 0.0F;
        break;
    case MOTOR_STATE_DRIVING:
        if (RTMButton_GetState()) {

            if (torqueDemand <= 0.0F &&
                MCU_GetMCU1Data()->motorDirection == MOTOR_DIRECTION_FORWARD) {
                // If regen is enabled and the torque demand is zero, we need to
                // set the torque demand to 0 to prevent the motor from applying
                // torque in the wrong direction
                motorData.desiredTorque = MAX_REGEN_TORQUE * REGEN_BIAS;
            } else {
                motorData.desiredTorque = torqueDemand;
            }

        } else {
            motorData.state = MOTOR_STATE_IDLE;
            torqueDemand = 0;
        }

        break;
    case MOTOR_STATE_FAULT:
        // RTMButton_Reset(); // force set RTM OFF
        motorData.desiredTorque = 0.0F;
        if (Faults_CheckAllClear()) {
            Motor_ClearFaultState();
        }
        break;
    default:
        break;
    }
}

#endif

#if HIMAC_FLAG
void Motor_UpdateMotor(float torqueDemand, bool enablePrecharge,
                       bool enablePower, bool enableRun, bool enableRegen,
                       bool enableStandby) {

    uint8_t prechargeState = PCC_GetData()->state;
    uint16_t prechargeProg = PCC_GetData()->prechargeProgress;
    // off --> standby --> precharge --> run --> fault -->standy
    // no kl15 then off
    switch (motorData.state) {
    case MOTOR_STATE_OFF: {
        if (enableStandby) {
            // # if HIMAC_FLAG
            //     Serial.println("Standby Mode");
            // #endif
            motorData.state = MOTOR_STATE_STANDBY;
        }
        motorData.desiredTorque = 0.0F;
        break;
    }
    // kl15 on and ready to go into precharge
    case MOTOR_STATE_STANDBY: {
        if (enablePrecharge) {

            motorData.state = MOTOR_STATE_PRECHARGING;
        }
        motorData.desiredTorque = 0.0F;
        break;
    }
    // HV switch on (PCC CAN message)
    case MOTOR_STATE_PRECHARGING: {
        if ((prechargeProg >= 94 && prechargeState == 3) || enablePower) {
            // # if HIMAC_FLAG
            //     Serial.println("Precharge finished");
            // # endif
            motorData.state = MOTOR_STATE_IDLE;
        } else if (enableStandby) {
            motorData.state = MOTOR_STATE_STANDBY;
        }

        motorData.desiredTorque = 0.0F;
        break;
    }
    // PCC CAN message finished
    case MOTOR_STATE_IDLE: {
        if (enableRun) {
            motorData.state = MOTOR_STATE_DRIVING;
        }
        motorData.desiredTorque = 0.0F;
        break;
    }
    // Ready to drive button pressed
    case MOTOR_STATE_DRIVING: {
        if (enableRun) {

            if (enableRegen && torqueDemand <= 0.0F &&
                MCU_GetMCU1Data()->motorDirection == MOTOR_DIRECTION_FORWARD) {
                // If regen is enabled and the torque demand is zero, we need to
                // set the torque demand to 0 to prevent the motor from applying
                // torque in the wrong direction
                motorData.desiredTorque = MAX_REGEN_TORQUE * REGEN_BIAS;
            } else {
                motorData.desiredTorque = torqueDemand;
            }

        } else {
            motorData.state = MOTOR_STATE_IDLE;
            torqueDemand = 0;
        }
        break;
    }
    // Any fault error occurs
    case MOTOR_STATE_FAULT: {

        if (enableStandby) {
            motorData.state = MOTOR_STATE_STANDBY;
        } else if (enableRun) {
            Motor_ClearFaultState();
        }
        motorData.desiredTorque = 0.0F;
        break;
    }
    default: {
        break;
    }
    }
}
#endif

float Motor_GetTorqueDemand() { return motorData.desiredTorque; }

void Motor_SetFaultState() { motorData.state = MOTOR_STATE_FAULT; }

void Motor_ClearFaultState() { motorData.state = MOTOR_STATE_IDLE; }

MotorState Motor_GetState() { return motorData.state; }
