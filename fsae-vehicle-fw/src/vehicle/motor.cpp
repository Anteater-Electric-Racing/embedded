// Anteater Electric Racing, 2025

#define STATE_MACHINE_STACK_SIZE 128
#define STATE_MACHINE_PRIORITY 2

#include <Arduino.h>
#include <arduino_freertos.h>
#include <algorithm>

#include "../utils/pid.h"
#include "../utils/regen.h"

#include "motor.h"
#include "apps.h"
#include "faults.h"
#include "telemetry.h"



typedef struct {
    MotorState state;
    float torqueDemand;
} MotorData;

MotorData motorData;
APPS apps;
BSE bse;
PIDConfig config;
TelemetryData *telemetry;

/**
 * 3/31 Task List:
 * 1) DONE (Consilidate Telemetry data)
 * 2) stateMachine Edge Cases
 * 3) remove redundant comments
 * 4) Implement Motor Torque commands
 * 5) Send for Review (4/2)
 */


void stateMachineTask(void *pvParameters);

void Motor_Init() {
    xTaskCreate(
        stateMachineTask,  // Task function
        "stateMachineTask",    // Task name
        STATE_MACHINE_STACK_SIZE,  // Stack size (words, not bytes)
        NULL,              // Task parameter
        STATE_MACHINE_PRIORITY, // Priority
        NULL  // Task handle
    );

    motorData.state = MOTOR_STATE_OFF;
}

 bool Motor_TransitionToPrecharging() {
    // can't transition to precharging if the motor is still running
    if (motorData.state != MOTOR_STATE_OFF) {
        return false;
    }
    // check if systems are ready
    if (telemetry->accumulatorVoltage < 10.0F ||
        telemetry->accumulatorTemp > 100.0F ||
        telemetry->tractiveSystemVoltage < 10.0F) { // TODO: change thresholds
        return false;
    }
    // transition to precharging after all checks are passed
    return true;
}

static bool Motor_TransitionToIdle() {
    // check that precharging is complete
    // low voltage on high voltage not on
    if (motorData.state != MOTOR_STATE_PRECHARGING) {
        return false;
    }
    // check precharge completion
    if (telemetry->tractiveSystemVoltage <
        10.0F) { // TODO: change threshold to reflect precharge completion
        return false;
    }
    return true;
}

static bool Motor_TransitionToDriving() {
    if (motorData.state != MOTOR_STATE_IDLE) {
        return false;
    }

    motorData.state = MOTOR_STATE_DRIVING;
    return true;
}

void stateMachineTask(void *pvParameters){
    /**TODO:
     * 1) Pull telemetry data here as a pointer - acess through whole class from here
     * 2) remove uneeded functions and put into each case
     */

    while (true){
        telemetry = Telemetry_GetData();
        vTaskDelay(pdMS_TO_TICKS(100));
        switch (motorData.state) // state transition conditions go here
        {
        case MOTOR_STATE_OFF:
            if (Motor_TransitionToPrecharging()){
                motorData.state = MOTOR_STATE_PRECHARGING;
            }
            break;
        case MOTOR_STATE_PRECHARGING:
            if (Motor_TransitionToIdle()){
                motorData.state = MOTOR_STATE_IDLE;
            }
            break;
        case MOTOR_STATE_IDLE:
            if(Motor_TransitionToDriving()){
                motorData.state = MOTOR_STATE_DRIVING;
            }
            break;
        case MOTOR_STATE_DRIVING:
            if (!Motor_CheckReadyToDrive()){
                motorData.state = MOTOR_STATE_FAULT;
            }
            break;
        case MOTOR_STATE_FAULT:
            break;
        default:
            motorData.state = MOTOR_STATE_OFF;
            break;
        }
    }
}

// handle state transitions
//TODO: call in ISR with ADC readings
void Motor_UpdateMotor() {
    switch (motorData.state) {
    case MOTOR_STATE_OFF:
    case MOTOR_STATE_PRECHARGING:
    case MOTOR_STATE_IDLE: {
        // check for ready to drive button press
        motorData.torqueDemand = 0.0F;
        break;
    }
    case MOTOR_STATE_DRIVING: {

        // Regen Braking Code (to be updated - has syntax errors)

        float currentSpeed = telemetry->vehicleSpeed;
        float initialSpeed = telemetry->initialSpeed;
        float brakePressure = bse.BSE_GetBSEReading();
        float torqueDemand = motorData.torqueDemand;
        float vehicleMass =
            180.0F; // TODO: change this to the actual mass. should include this
                    // directly in the regen file or on the top level

        // only applied when braking
        if (brakePressure > REGEN_BRAKE_THRESHOLD) { // If brakes are applied
            float regenTorque =
                Motor_CalculateRegenBraking(brakePressure, currentSpeed);
                                            //vehicleMass, initialSpeed,torqueDemand);
                // apply to rear wheels
                Motor_SetRearMotorTorque(regenTorque);
        }

        break;
    }
    case MOTOR_STATE_FAULT: {
        motorData.torqueDemand = 0.0F;
        break;
    }
    default: {
        motorData.torqueDemand = 0.0F;
        break;
    }
    }
}

// helper functions
float Motor_DirectTorqueControl(float &maxToruqe) {
    return apps.APPS_GetAPPSReading() *
           maxToruqe; // make maxTorque a constant under utils section
}

float Motor_TorqueTractionControl(float &target_slip, float &current_slip,
                                  float &dt) {
    return PID::PID_Calculate(config, target_slip, current_slip, dt);
}

float Motor_GetTorqueDemand() { return motorData.torqueDemand; }

float Motor_SetRearMotorTorque(float regenTorque) {
    if (regenTorque > 0)
        regenTorque = -regenTorque;

    // TODO: create function to apply regen torque to back wheels
    Motor_SetRearMotorTorque(regenTorque);
}
void Motor_SetFaultState() { motorData.state = MOTOR_STATE_FAULT; }

bool Motor_CheckReadyToDrive() {
    // check if faults are clear
    if (!Faults_CheckAllClear() || !Motor_CheckReadyToDrive()) {
        Motor_SetFaultState();
        return false;
    }
    // check if systems are ready
}
