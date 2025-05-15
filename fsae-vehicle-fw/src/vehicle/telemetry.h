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

    // MCU1 data
    float motorSpeed; // Motor speed in RPM
    float motorTorque; // Motor torque in Nm
    float maxMotorTorque; // Max motor torque in Nm
    float maxMotorBrakeTorque; // Max motor brake torque in Nm
    float motorDirection; // Motor direction
    float mcuMainState; // Motor main state
    float mcuWorkMode; // MCU work mode

    // MCU2 data
    float motorTemp; // Motor temperature in C
    float mcuTemp; // Hardware temperature in C
    bool dcOverVoltFault; // DC over voltage fault
    bool motorOvrHotFault; // Motor over heat fault
    bool motorOvrSpdFault; // Motor over speed fault
    bool phaseCurSensorFault; // Phase current sensor fault
    bool resolverFault; // Resolver fault
    bool overHotFault; // Over hot fault
    bool motorPhaseCurFault; // Motor phase current fault
    bool dcOverCurFault; // DC over current fault
    bool v12LowVoltFault; // V12 low voltage fault
    bool dcLowVoltFault; // DC low voltage fault


    float debug[4]; // Debug data
} TelemetryData;

void Telemetry_Init();
TelemetryData const* Telemetry_GetData();
void Telemetry_UpdateData(TelemetryData* data);

