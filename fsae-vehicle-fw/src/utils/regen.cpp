// Anteater Electric Racing, 2025

#include "regen.h"
#include <math.h>

/**
 * https://www.sciencedirect.com/science/article/pii/S2352484722021941#:~:text=The%20brake%20regeneration%20energy%20can,85%20%E2%88%BC%200%20.
 * Formula for energy transfer: E = integral(Fv/n)
 * F = force applied to wheels (braking force)
 * v = speed of the vehicle
 * n = energy charging efficiency
 */

// determine how much torque should be applied
//? Are all of these parameters necessary?
float Motor_CalculateRegenBraking(float brakePosition, float currentSpeed, float mass, float initialSpeed) {
    //TODO: check over the formula
    float wheelRadius = 0.3302; //TODO: get actual wheel radius

    if (currentSpeed > MAX_REGEN_SPEED) {
        // apply braking and friction
    };

    if (brakePosition < REGEN_BRAKE_THRESHOLD) {
        // do not use regn if brake is not powerful enough
        return 0.0F;
    };

    // calculate energy loss
    float energyLoss = 1/2*mass*(pow(currentSpeed, 2) - pow(initialSpeed, 2));

    // calculate available regen energy
    float availableRegenEnergy = energyLoss * 0.85; //TODO: change constant to actual efficiency of braking system

    // calculate regen torque
    float regenTorque = availableRegenEnergy / wheelRadius;

    // check that torque does not exceed threshold
    if (regenTorque > MAX_REGEN_TORQUE) {
        return MAX_REGEN_TORQUE;
    };

    return regenTorque;
}