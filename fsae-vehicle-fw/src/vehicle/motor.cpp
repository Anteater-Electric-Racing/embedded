// Anteater Electric Racing, 2025

#include "vehicle/motor.h"
#include "vehicle/telemetry.h"
#include "vehicle/faults.h"

typedef struct{
    MotorState state;
    float torqueDemand;
} MotorData;

MotorData motorData;

void Motor_Init(){
    motorData.state = MOTOR_STATE_OFF;
}
// state transition functions
static bool Motor_TransitionToPrecharging() {
    // can't transition to precharging if the motor is still running
    if (motorData.state != MOTOR_STATE_OFF) {
        return false;
    }

    // check if systems are ready
    TelemetryData* telemetry = Telemetry_GetData();
    if (telemetry->accumulatorVoltage < 10.0F || telemetry->accumulatorTemp > 100.0F || telemetry->tractiveSystemVoltage < 10.0F) {
        return false;
    }

    // check if faults are clear
    if (!Faults_CheckAllClear()) {
        return false;
    }

    // motor is idle and ready to precharge
    motorData.state = MOTOR_STATE_IDLE;
    return true;
}

// handle state transitions
void Motor_UpdateMotor(){
    switch(motorData.state){
        case MOTOR_STATE_OFF:
        {
            motorData.torqueDemand = 0.0F;

            break;
        }
        case MOTOR_STATE_PRECHARGING:
        {
            motorData.torqueDemand = 0.0F;
            break;
        }
        case MOTOR_STATE_IDLE:
        {
            // check for ready to drive button press
            motorData.torqueDemand = 0.0F;
            break;
        }
        case MOTOR_STATE_DRIVING:
        {
            // apps.getThrottle() from accelerator pedal sensor for setpoint maybe?
            // add safety checks
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

