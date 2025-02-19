// Anteater Electric Racing, 2025

#include "regen.h"
#include "faults.h"
#include <math.h>

float Motor_CalculateRegenBraking(float brakePressure, float currentSpeed, float mass, float initialSpeed, float torqueDemand) {
    //TODO: check over the formula
    float wheelRadius = 0.3302; //TODO: get actual wheel radius

    if (_Motor_EnableRegenBraking(currentSpeed, brakePressure, torqueDemand)) {
        // proportion of braking force for regen
        float regenPercentage = fminf(MAX_REGEN_PERCENTAGE, brakePressure);
        
        // kinetic energy loss
        float energyLoss = 1/2*mass*(pow(currentSpeed, 2) - pow(initialSpeed, 2));

        // calculate available regen energy
        float availableRegenEnergy = energyLoss * 0.85; //TODO: change constant to actual efficiency of braking system

        // calculate regen torque to apply
        float regenTorque = (availableRegenEnergy / wheelRadius) * regenPercentage;
 
        // check limit
        regenTorque = fminf(regenTorque, MAX_REGEN_TORQUE);

        // only apply on back wheels
        float rearRegenTorque = regenTorque / 2;

        return rearRegenTorque;
    }

    return 0.0F;
}

/**
 * Conditions to enable regen braking:
 * - speed is within regen range (under MAX_REGEN_SPEED)
 * - brake force is above a threshold (if > threshold then use regen)
 * - torque can be handled by motor (if < MAX_REGEN_TORQUE)
 */
bool _Motor_EnableRegenBraking(float currentSpeed, float brakePosition, float torqueDemand) {
    // do not enable regen
    if (currentSpeed > MAX_REGEN_SPEED // speed is too high
        || brakePosition < REGEN_BRAKE_THRESHOLD // brake is not powerful enough
        || torqueDemand > MAX_REGEN_TORQUE
        || !Faults_CheckAllClear()) { // torque is too high

        return false;
    };

    return true;
}