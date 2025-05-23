// Anteater Electric Racing, 2025

#ifndef TELEMETRY_H
#define TELEMETRY_H

typedef struct {
    float APPS_Travel; // APPS travel in %

    float BSEFront_PSI; // front brake pressure in PSI
    float BSERear_PSI; // rear brake pressure in PSI

    float accumulatorVoltage = 0.0f;
    float accumulatorTemp = 0.0f;

    float tractiveSystemVoltage = 0.0f;

    MotorState motorState; // motor state
} TelemetryData;

void Telemetry_Init();
bool Telemetry_GetData(TelemetryData& data);
void Telemetry_UpdateData(const TelemetryData& data);

#endif
