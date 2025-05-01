// Anteater Electric Racing, 2025
#pragma once

#include <stdint.h>

#include "motor.h"

typedef struct {
    float APPS_Travel; // APPS travel in %

    float BSEFront_PSI; // front brake pressure in PSI
    float BSERear_PSI; // rear brake pressure in PSI

    float accumulatorVoltage;
    float accumulatorTemp_F;

    float tractiveSystemVoltage;

    MotorState motorState; // Motor state

    float debug[4]; // Debug data
} TelemetryData;

void Telemetry_Init();
TelemetryData const* Telemetry_GetData();
void Telemetry_UpdateData(TelemetryData* data);

