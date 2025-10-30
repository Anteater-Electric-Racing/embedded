// Testing Launch Control System by Rishi and Anoop

#include <Arduino.h>
#include <algorithm>
#include <arduino_freertos.h>
#include "launch.h"
#include <telemetry.h>
#include <vehicle/motor.h>  // Not used but maybe could be usedful ?
#include "bse.h"
#include "apps.h"
#include "faults.h"
#include "ifl100-36.h"

#include "../utils/pid.h"


static PIDConfig slipPIDConfig;
static float slipTarget = 0.07f;   // 7% slip
static float minTorque = 0.0f;
static float wheelRadius = 0.5f; // in meters, adjust based on actual wheel radius

static LaunchState launchControlState = LAUNCH_STATE_OFF;

void LaunchControl_Init()
{
    //Pls tune these values properly I am just guessing them for this 
    slipPIDConfig.kP = 50.0f; 
    slipPIDConfig.kI = 5.0f;
    slipPIDConfig.kD = 0.0f;
    slipPIDConfig.kS = 0.0f;
    slipPIDConfig.kV = 0.0f;
    slipPIDConfig.max = 260.0f; // Max motor torque in Nm according to Utils.h
    slipPIDConfig.min = 0.0f;
    slipPIDConfig.integral_max = 20.0f;
    slipPIDConfig.integral_min = -20.0f;
    PID::PID_Reset();

    // Initialize PID control parameters and *maybe* set default state at false for driver choice
}

float threadLaunchControl(void *pvParameters)
{
    float wheelSpeedFL = 0.0f; // placeholder
    float wheelSpeedFR = 0.0f; // placeholder
    float torqueDemand = 0.0f;

    switch (launchControlState) {
        case LAUNCH_STATE_ON:
            float realTorque = Motor_GetState()->torqueCmd; // Current torque command from motor controller
            //This is assuming we are obtaining wheel speeds in rad/s

            float controlledSpeed = MCU_GetMCU1Data()->motorSpeed * wheelRadius; // Obtain the higher speed of the wheels connected to Powertrain for safety precaution
            float freeSpeed = std::min(wheelSpeedFL * wheelRadius, wheelSpeedFR * wheelRadius);
            if(freeSpeed == 0.0f) // Free roaming wheels (front two) and take the lower speed of these for safety precaution
            {
                freeSpeed = 0.001f; // To avoid division by zero
            }
            float slipRatio = (controlledSpeed - freeSpeed) / freeSpeed; //Slip Ratio equation provided by Vik

            // if(slipRatio < 0.05f || slipRatio > 0.15f)
            // {        //Doubt this is needed, as PID should be able to handle this
            // }
            

            float maxTorqueNm = Telemetry_GetData()->maxMotorTorque; // Just to understand Max Torque so we don't exceed this
            float dt = 0.01f; // Time step for PID calculation, adjusted for 100Hz update rate
            
            // PID Control for Slip Ratio
            float correction = PID::PID_Calculate(slipPIDConfig, slipTarget, slipRatio, dt); // Proportional control for slip ratio
            // Currently unsure how this will be integrated, as the correction should reduce torque when slip is high and increase when low, but unsure what values would come out
            torqueDemand = realTorque + correction; // Reduce torque based on slip ratio correction


            // limit torque demand to be within allowable limits(no less then 0 and no more the nmaxToqrue which I believe is 260 according to Utils.h)
            if(torqueDemand > maxTorqueNm)
            {
                torqueDemand = maxTorqueNm;
            }
            else if(torqueDemand < minTorque)
            {
                torqueDemand = minTorque;
            }
            

            if (std::max(BSE_GetBSEReading()->bseFront_PSI, BSE_GetBSEReading()->bseRear_PSI) > 50.0f) {
                // If brake is pressed, disable launch control
                PID::PID_Reset();
                launchControlState = LAUNCH_STATE_OFF;
            }

            if (APPS_GetAPPSReading() < 1.0f) {
                PID::PID_Reset();
                launchControlState = LAUNCH_STATE_OFF;
            }
            break;
            
        case LAUNCH_STATE_OFF:
            // Implement launch control logic here
            if ((std::max(BSE_GetBSEReading()->bseFront_PSI, BSE_GetBSEReading()->bseRear_PSI) > 50.0f) && (MCU_GetMCU1Data()->motorSpeed == 0.0f) && Motor_GetState() == MOTOR_STATE_DRIVING) {     
                launchControlState = LAUNCH_STATE_ON;   //If Car isn't moving and brake is pressed, enable launch control to active
            }
            break;
        case LAUNCH_STATE_FAULT:
            // Handle fault state
            PID::PID_Reset();
            torqueDemand = 0.0f;
            launchControlState = LAUNCH_STATE_OFF;
            Faults_SetFault(FAULT_LAUNCH_CONTROL);
            break;
        }
    return torqueDemand;
}

LaunchState Launch_getState() {
    return launchControlState;
}