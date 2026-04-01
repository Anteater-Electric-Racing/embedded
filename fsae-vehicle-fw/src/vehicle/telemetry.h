// Anteater Electric Racing, 2025
#pragma once

#include <stdint.h>

#include "peripherals/adc.h"
#include "peripherals/can.h"
#include "shockTravel.h"
#include "utils/utils.h"
#include "vehicle/apps.h"
#include "vehicle/bse.h"
#include "vehicle/faults.h"
#include "vehicle/ifl100-36.h"
#include "vehicle/motor.h"

typedef struct __attribute__((packed)) {

    // Analog Data
    float APPS_Travel; // APPS travel in %

    float BSEFront; // front brake pressure in PSI
    float BSERear;  // rear brake pressure in PSI

    float imdResistance;
    uint32_t imdStatus;

    // BMS Data
    float packVoltage;
    float packCurrent;
    float soc;
    float dischargeLimit;
    float chargeLimit;
    float lowCellVolt;  // Volts (e.g., 3.4215f)
    float highCellVolt; // Volts
    float avgCellVolt;  // Volts

    // MCU1 data
    float motorSpeed;     // Motor speed in RPM
    float motorTorque;    // Motor torque in Nm
    float maxMotorTorque; // Max motor torque in Nm
    // float maxMotorBrakeTorque; // Max motor brake torque in Nm
    MotorRotateDirection motorDirection; // Motor direction
    MotorState motorState;

    MCUMainState mcuMainState; // Motor main state
    MCUWorkMode mcuWorkMode;   // MCU work mode

    float mcuVoltage;
    float motorPhaseCurrent;
    float mcuCurrent;

    // MCU2 data
    int32_t motorTemp; // Motor temperature in C
    int32_t mcuTemp;   // Inverter temperature in C

    MCUWarningLevel mcuWarningLevel; // MCU warning level

    // Dynamics Data
    float shocktravel1;
    float shocktravel2;
    float shocktravel3;
    float shocktravel4;

    bool dcMainWireOverVoltFault;
    bool motorPhaseCurrFault;
    bool mcuOverHotFault;
    bool resolverFault;
    bool phaseCurrSensorFault;
    bool motorOverSpdFault;
    bool drvMotorOverHotFault;
    bool dcMainWireOverCurrFa;
    bool drvMotorOverCoolFaul;
    bool dcLowVoltWarning;
    bool mcu12VLowVoltWarning;
    bool motorStallFault;
    bool motorOpenPhaseFault;

    int32_t faultMap; // Debug data
} TelemetryData;

void Telemetry_Init();
void threadTelemetry(void *pvParameters);
TelemetryData const *Telemetry_GetData();
