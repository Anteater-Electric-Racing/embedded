// Anteater Electric Racing, 2025

#define SPEED_CONTROL_ENABLED 0
#define SPEED_P_GAIN 0.01F // Proportional gain for speed control
#define SPEED_I_GAIN 0.1F  // Integral gain for speed control

#define LOW_VOLT_LIMIT 2.3F

#include "peripherals/can.h"
#include "peripherals/gpio.h"
#include "utils/utils.h"
#include <arduino_freertos.h>

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

// Define 3 Presets (Steepness k, Midpoint x0)
// Map 0: Rain (High precision, late power)
// Map 1: Endurance (Balanced, predictable)
// Map 2: Autocross  (More linear + induce level shift)

/*

intial default k = 9.0, x0 = 0.35
karan/gabenote: gets to speed too qucik (nears max at 60% pedal travel)
3/11 change (tbd): x0 = 0.425 (nears max at 75% pedal travel)

*/
const float k_vals[] PROGMEM = {10.0f, 9.0f, 12.0f};
const float x0_vals[] PROGMEM = {0.7f, 0.425f, 0.375f};

float k = 0.0f, x0 = 0.0f, low_limit = 0.0f, high_limit = 0.0f;

float targetTorque = 0.0F;

void Motor_Init() {
    motorData.state = MOTOR_STATE_PRECHARGING; // DEFAULT TO PRECHARGE
    motorData.desiredTorque = 0.0F;            // No torque demand at start

    k = k_vals[ACTIVE_MAP];
    x0 = x0_vals[ACTIVE_MAP];

    low_limit = 1.0f / (1.0f + expf(-k * (0.0f - x0)));
    high_limit = 1.0f / (1.0f + expf(-k * (1.0f - x0)));
}

void threadMotor(void *pvParameters) {
    xLastWakeTime = xTaskGetTickCount();
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
            vcu1.VCU_MotorMode = 1; // ? 1 : 2; // 0 = Standby, 1 = Drive, 2 =
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

        static float lastTorqueSent = 0.0f;

        float torqueDelta = targetTorque - lastTorqueSent;
        // Apply Deadband
        if (APPS_GetAPPSReading() > 0.03f) {
            targetTorque = torqueMap(APPS_GetAPPSReading());
        }

        // might be very redundant
        if (torqueDelta < -MAX_TORQUE_STEP_DOWN_PCT) { //
            targetTorque = lastTorqueSent - MAX_TORQUE_STEP_DOWN_PCT;
        } else if (APPS_GetAPPSReading() <= 0.03f) {
            targetTorque = 0;
        }

        /*safety check if no regen at all*/
        if (targetTorque <= 0) {
            targetTorque = 0;
        }

        lastTorqueSent = targetTorque;

        // float pedalTorque;
        // float p = APPS_GetAPPSReading1();
        // float TmaxCmd = (MOTOR_MAX_TORQUE * 0.5F);

        // // PIECEWISE pedal map
        // // Breakpoints
        // const float p1 = 0.10F;
        // const float p2 = 0.50F;

        // // Relative slope scalars
        // const float K_LOW = 1.2;
        // const float K_MID = 3.42F;
        // const float K_HIGH = 2.4F;

        // // best constants: KLOW = 0.35, KMID = 1.0, K_HIGH = 0.7

        // // Compute mid slope so that p=1.0 -> TmaxCmd (keeps same top-end
        // as
        // // before)
        // float denom =
        //     (K_LOW * p1) + (K_MID * (p2 - p1)) + (K_HIGH * (1.0F - p2));
        // float m2 = TmaxCmd / denom;

        // float m1 = K_LOW * m2;
        // float m3 = K_HIGH * m2;

        // if (p <= p1) {
        //     pedalTorque = m1 * p + 5.0F;
        // } else if (p <= p2) {
        //     float T1 = m1 * p1;
        //     pedalTorque = T1 + m2 * (p - p1) + 5.0F;
        // } else {
        //     float T1 = m1 * p1;
        //     float T2 = T1 + m2 * (p2 - p1);
        //     pedalTorque = T2 + m3 * (p - p2) + 5.0F;
        // }

        // Linear Map Torque
        // if (APPS_GetAPPSReading1() > 0.06) {
        //     pedalTorque = APPS_GetAPPSReading1() * (MOTOR_MAX_TORQUE *
        //     0.2F);
        // } else {
        //     pedalTorque = 0;
        // }

#if !HIMAC_FLAG
        Motor_UpdateMotor((targetTorque));
#endif

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(5));
    }
}

#if !HIMAC_FLAG
void Motor_UpdateMotor(float torqueDemand) {

    Faults_HandleFaults();
    // if (BMS_GetOrionData()->lowCellVolt > LOW_VOLT_LIMIT) {
    //     Faults_ClearFault(LOW_BATTERY_VOLTAGE_FAULT);
    // }
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
        // transition to IDLE
        // TODO Update brake light threshold if we only want to move when
        // mech brakes are engaged
        if (BSE_GetBSEReading()->bseFront_Reading >= BRAKE_LIGHT_THRESHOLD &&
            BSE_GetBSEReading()->bseRear_Reading >= BRAKE_LIGHT_THRESHOLD) {
            if (RTMButton_GetState() && Faults_CheckAllClear()) {
                motorData.state = MOTOR_STATE_DRIVING;
            }
        } else {
            RTMButton_Reset();
        }
        motorData.desiredTorque = 0.0F;
        break;
    case MOTOR_STATE_DRIVING:
        if (RTMButton_GetState()) {

            // if (BMS_GetOrionData()->lowCellVolt < LOW_VOLT_LIMIT) {
            //     Faults_SetFault(LOW_BATTERY_VOLTAGE_FAULT);
            // }
            // if (torqueDemand <= 0.0F &&
            //     MCU_GetMCU1Data()->motorDirection == MOTOR_DIRECTION_FORWARD)
            //     {
            //     // If regen is enabled and the torque demand is zero, we
            //     // need to set the torque demand to 0 to prevent the motor
            //     // // from applying torque in the wrong direction
            //     // motorData.desiredTorque = MAX_REGEN_TORQUE * REGEN_BIAS;
            // } else {
            motorData.desiredTorque = torqueDemand;
            // }

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
                // If regen is enabled and the torque demand is zero, we
                // need to set the torque demand to 0 to prevent the motor
                // from applying torque in the wrong direction
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

float torqueMap(float pedal) {
    // Raw Sigmoid curve
    float raw = 1.0f / (1.0f + expf(-k * (pedal - x0)));
    float normalized_ratio = (raw - low_limit) / (high_limit - low_limit);
    float target = (normalized_ratio * CAPPED_MOTOR_TORQUE);
    return target;
}

float Motor_GetTorqueDemand() { return motorData.desiredTorque; }

void Motor_SetFaultState() { motorData.state = MOTOR_STATE_FAULT; }

void Motor_ClearToIdleFault() {
    motorData.state = MOTOR_STATE_FAULT;
    RTMButton_Reset();
}

void Motor_ClearFaultState() { motorData.state = MOTOR_STATE_DRIVING; }

MotorState Motor_GetState() { return motorData.state; }

float Motor_TargetTorque() { return targetTorque; }
