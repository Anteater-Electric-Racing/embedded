// Anteater Electric Racing, 2025
#ifndef REGEN_H
#define REGEN_H

// TODO: change these values
#define MAX_REGEN_TORQUE 50.0F // rotational force applied to wheels (prevent excessive braking)
#define REGEN_BRAKE_THRESHOLD 10.0F // minimum braking force to apply regen (determine when regen is used)
#define MAX_REGEN_SPEED 100.0F // maximum speed to apply regen ( prevent use of regen at high speeds )

/**
 * https://www.sciencedirect.com/science/article/pii/S2352484722021941#:~:text=The%20brake%20regeneration%20energy%20can,85%20%E2%88%BC%200%20.
 * Formula: E = integral(Fv/n)
 * F = force applied to wheels (braking force)
 * v = speed of the vehicle
 * n = energy charging efficiency
 */
float Motor_CalculateRegenBraking(float brakePosition, float currentSpeed);

bool Motor_EnableRegenBraking(float currentSpeed);
#endif