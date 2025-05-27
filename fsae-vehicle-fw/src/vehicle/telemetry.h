// Anteater Electric Racing, 2025
#pragma once

#include <stdint.h>
#include "motor.h"
#include <peripherals/adc.h>

struct TelemetryData{
    float APPS_Travel; // APPS travel in %

    float BSEFront_PSI; // front brake pressure in PSI
    float BSERear_PSI; // rear brake pressure in PSI

    float accumulatorVoltage;
    float accumulatorTemp_F;

    float tractiveSystemVoltage;

    MotorState motorState; // Motor state

    float debug[4]; // Debug data
    uint16_t adc0Reads[SENSOR_PIN_AMT_ADC0];
    uint16_t adc1Reads[SENSOR_PIN_AMT_ADC1];

} __attribute__((packed)); // need this to ensure data is packed without gaps in between;

void Telemetry_Init();
void threadTelemetryCAN(void *pvParameters);
TelemetryData const* Telemetry_GetData();
void Telemetry_SerializeData(TelemetryData data, uint8_t* serializedTelemetryBuf);
void Telemetry_UpdateData(TelemetryData* data);
void Telemetry_UpdateADCData(volatile uint16_t* adc0reads, volatile uint16_t* adc1reads);


