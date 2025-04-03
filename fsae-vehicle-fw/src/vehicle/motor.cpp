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
 * 2) DONE stateMachine full path of execution
 * 3) DONE Implement Motor Torque commands
 * 4) Put constants in motor.h file, update all functions in motor.h file
 * 5) Send for Review (4/4)
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
     while (true) {
        telemetry = Telemetry_GetData();
        vTaskDelay(pdMS_TO_TICKS(100));

        switch (motorData.state) {
            case MOTOR_STATE_OFF:
                if (Motor_TransitionToIdle()) {
                    motorData.state = MOTOR_STATE_IDLE;
                }
                break;

            case MOTOR_STATE_IDLE:
                if (!Faults_CheckAllClear()) {
                    motorData.state = MOTOR_STATE_FAULT;
                } else if (Motor_TransitionToPrecharging()) {
                    motorData.state = MOTOR_STATE_PRECHARGING;
                }
                break;

            case MOTOR_STATE_PRECHARGING:
                if (!Faults_CheckAllClear()) {
                    motorData.state = MOTOR_STATE_FAULT;
                } else if (Motor_TransitionToDriving()) {
                    motorData.state = MOTOR_STATE_DRIVING;
                }
                break;

            case MOTOR_STATE_DRIVING:
                if (!Faults_CheckAllClear()) {
                    motorData.state = MOTOR_STATE_FAULT;
                }
                break;

            case MOTOR_STATE_FAULT:
                // Handle fault state, combine with fault module
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


        //review this 4/2
        float target = 0;
        float current = 0;
        float dt = 0;
        motorData.torqueDemand = std::min(Motor_DirectTorqueControl(20.0F), Motor_TorqueTractionControl(target,current,dt));

        float currentSpeed = telemetry->vehicleSpeed;
        float initialSpeed = telemetry->initialSpeed;
        float brakePressure = bse.BSE_GetBSEReading();
        float vehicleMass = 180.0F; // TODO: change this to the actual mass. should include this
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
float Motor_DirectTorqueControl(float const &maxTorque) {
    return apps.APPS_GetAPPSReading() *
           maxTorque; // make maxTorque a constant under utils section
}

float Motor_TorqueTractionControl(float &target_value, float &current_value,
                                  float &dt) {
    return PID::PID_Calculate(config, target_value, current_value, dt);
}

float Motor_GetTorqueDemand() { return motorData.torqueDemand; }

float Motor_SetRearMotorTorque(float regenTorque) {
    if (regenTorque > 0)
        regenTorque = -regenTorque;

    // TODO: create function to apply regen torque to back wheels
    Motor_SetRearMotorTorque(regenTorque);
}
// void Motor_SetFaultState() { motorData.state = MOTOR_STATE_FAULT; }


// bool Motor_CheckReadyToDrive() {
//     // check if faults are clear
//     if (!Faults_CheckAllClear() || !Motor_CheckReadyToDrive()) {
//         return false;
//     }
// }
