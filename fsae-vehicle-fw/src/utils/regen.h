// Anteater Electric Racing, 2025
#ifndef REGEN_H
#define REGEN_H

// TODO: change these values once we test
#define MAX_REGEN_TORQUE 50.0F // rotational force applied to wheels (prevent excessive braking)
#define REGEN_BRAKE_THRESHOLD 10.0F // minimum braking force to apply regen (determine when regen is used)
#define MAX_REGEN_SPEED 100.0F // maximum speed to apply regen ( prevent use of regen at high speeds )
#define MAX_REGEN_PERCENTAGE 0.7F // 70% of the brake force


float Motor_CalculateRegenBraking(float brakePosition, float currentSpeed );

bool Motor_EnableRegenBraking(float currentSpeed);
#endif