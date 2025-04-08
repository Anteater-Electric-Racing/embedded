// Anteater Electric Racing, 2025

#ifndef TELEMETRY_H
#define TELEMETRY_H

typedef struct {
    float APPS_Travel; // APPS travel in %

    float BSEFront_PSI; // front brake pressure in PSI
    float BSERear_PSI; // rear brake pressure in PSI

    float accumulatorVoltage;
    float accumulatorTemp_F;

    float tractiveSystemVoltage;
} TelemetryData;

void Telemetry_Init();
TelemetryData const* Telemetry_GetData();
void Telemetry_UpdateData(TelemetryData* data);

#endif
