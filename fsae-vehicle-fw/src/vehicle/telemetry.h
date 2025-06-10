// Anteater Electric Racing, 2025
#pragma once

#include <stdint.h>

#include "vehicle/motor.h"
#include "peripherals/adc.h"
#include "vehicle/ifl100-36.h"

typedef struct __attribute__((packed)){
    float APPS_Travel; // APPS travel in %

    float BSEFront_PSI; // front brake pressure in PSI
    float BSERear_PSI; // rear brake pressure in PSI

    float accumulatorVoltage;
    float accumulatorTemp_F;

    MotorState motorState; // Motor state

    // MCU1 data
    float motorSpeed; // Motor speed in RPM
    float motorTorque; // Motor torque in Nm
    float maxMotorTorque; // Max motor torque in Nm
    float maxMotorBrakeTorque; // Max motor brake torque in Nm
    MotorRotateDirection motorDirection; // Motor direction
    MCUMainState mcuMainState; // Motor main state
    MCUWorkMode mcuWorkMode; // MCU work mode

    // MCU2 data
    int32_t motorTemp; // Motor temperature in C
    int32_t mcuTemp; // Inverter temperature in C
    bool dcMainWireOverVoltFault; // DC over voltage fault
    bool motorPhaseCurrFault; // MCU motor phase current fault
    bool mcuOverHotFault; // MCU overheat fault
    bool resolverFault; // Resolver fault
    bool phaseCurrSensorFault; // Phase current sensor fault
    bool motorOverSpdFault; // MCU motor over speed fault
    bool drvMotorOverHotFault; // Driver motor overheat fault
    bool dcMainWireOverCurrFault; // DC main wire over voltage fault
    bool drvMotorOverCoolFault; // Driver motor overcool fault
    bool mcuMotorSystemState; // MCU motor system state
    bool mcuTempSensorState; // MCU temperature sensor state
    bool motorTempSensorState; // MCU motor temperature sensor state
    bool dcVoltSensorState; // MCU DC voltage sensor state
    bool dcLowVoltWarning; // MCU DC low voltage warning
    bool mcu12VLowVoltWarning; // MCU 12V low voltage warning
    bool motorStallFault; // MCU motor stall fault
    bool motorOpenPhaseFault; // MCU motor open phase fault
    MCUWarningLevel mcuWarningLevel; // MCU warning level

    // MCU3 data
    float mcuVoltage; // DC main wire voltage in V
    float mcuCurrent; // DC main wire current in A
    float motorPhaseCurr; // Motor phase current in A

    float debug[4]; // Debug data
} TelemetryData;

void Telemetry_Init();
void threadTelemetryCAN(void *pvParameters);
TelemetryData const* Telemetry_GetData();
