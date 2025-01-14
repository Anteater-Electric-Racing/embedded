// Anteater Electric Racing, 2025

#ifndef TELEMETRY_H
#define TELEMETRY_H

typedef struct {
    float accumulatorVoltage;
    float accumulatorTemp;

    float tractiveSystemVoltage;
} TelemetryData;

void Telemetry_Init();
TelemetryData* Telemetry_GetData();

#endif
