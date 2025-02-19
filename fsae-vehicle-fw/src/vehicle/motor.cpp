// Anteater Electric Racing, 2025

#include "motor.h"
#include "telemetry.h"
#include "faults.h"
#include "../utils/pid.h"
#include "apps.h"
#include <algorithm>

typedef struct{
    MotorState state;
    float torqueDemand;
} MotorData;

MotorData motorData;
APPS apps;
PIDConfig config;

void Motor_Init(){
    motorData.state = MOTOR_STATE_OFF;
}

// state transition functions (check if conditions are met to initiate transition)
static bool Motor_TransitionToPrecharging() {
    // can't transition to precharging if the motor is still running
    if (motorData.state != MOTOR_STATE_OFF) {
        return false;
    }

    // check if systems are ready
    TelemetryData* telemetry = Telemetry_GetData(); // TODO: might not be ideal to pull data each time we transition to another state
    if (telemetry->accumulatorVoltage < 10.0F || telemetry->accumulatorTemp > 100.0F || telemetry->tractiveSystemVoltage < 10.0F) { // TODO: change thresholds
        return false;
    }

    // transition to precharging after all checks are passed
    motorData.state = MOTOR_STATE_PRECHARGING; // TODO: check the state dependencies
    return true;
}

static bool Motor_TransitionToIdle() {
    // check that precharging is complete
    // low voltage on high voltage not on
    if (motorData.state != MOTOR_STATE_PRECHARGING) {
        return false;
    }

    // check precharge completion
    TelemetryData* telemetry = Telemetry_GetData();
    if (telemetry->tractiveSystemVoltage < 10.0F) { // TODO: change threshold to reflect precharge completion
        return false;
    }

    // transition to idle state after precharging
    motorData.state = MOTOR_STATE_IDLE; // TODO: check the state dependencies
    return true;
}

static bool Motor_TransitionToDriving() {
    if (motorData.state != MOTOR_STATE_IDLE) {
        return false;
    }

    motorData.state = MOTOR_STATE_DRIVING;
    return true;
}

// handle state transitions
//TODO: add freeRTOS tasks for this
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
            TelemetryData* telemetry = Telemetry_GetData();
            float currentSpeed = telemetry->vehicleSpeed
            float initialSpeed = telemetry->initialSpeed
            float brakePressure = BSE_GetBSEReading();
            float torqueDemand = motorData.torqueDemand;
            float vehicleMass = 180.0F // TODO: change this to the actual mass. should include this directly in the regen file or on the top level

            // only applied when braking
           if (brakePressure > REGEN_BRAKE_THRESHOLD) { // If brakes are applied
                float regenTorque = Motor_CalculateRegenBraking(
                    brakePressure,
                    currentSpeed,
                    vehicleMass,
                    initialSpeed,
                    torqueDemand
                )

                // apply to rear wheels
                Motor_SetRearMotorTorque(regenTorque);
           }
            // apps.getThrottle() from accelerator pedal sensor for setpoint maybe?
            // add safety checks
            // pid.compute() for torque demand
         //   motorData.torqueDemand = std::min(
          //      Motor_DirectTorqueControl(0.0F),
          //      Motor_TorqueTractionControl(0.0F,0.0F,0.0F)); //dt comes from freeRTOS ticks
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

// helper functions
float Motor_DirectTorqueControl(float &maxToruqe){
    return apps.APPS_GetAPPSReading() * maxToruqe; //make maxTorque a constant under utils section
}

float Motor_TorqueTractionControl(float &target_slip, float &current_slip, float &dt){
    return PID::PID_Calculate(config, target_slip, current_slip, dt);
}

float Motor_GetTorqueDemand(){
    return motorData.torqueDemand;
}

float Motor_SetRearMotorTorque(float regenTorque){
    if (regenTorque > 0) 
        regenTorque = -regenTorque;
    
    // TODO: create function to apply regen torque to back wheels
    Motor_SetRearMotorTorque(regenTorque);
}
void Motor_SetFaultState(){
    motorData.state = MOTOR_STATE_FAULT;
}

bool Motor_CheckReadyToDrive() {
    // check if faults are clear
    if (!Faults_CheckAllClear() || !Motor_CheckReadyToDrive()) {
        Motor_SetFaultState();
        break;
    }
    // check if systems are ready
    Tele
}