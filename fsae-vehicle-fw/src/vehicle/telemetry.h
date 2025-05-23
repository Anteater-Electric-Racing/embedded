// Anteater Electric Racing, 2025

#ifndef TELEMETRY_H
#define TELEMETRY_H

typedef struct {
    float accumulatorVoltage = 0.0f;
    float accumulatorTemp = 0.0f;

    float tractiveSystemVoltage = 0.0f;
} TelemetryData;

void Telemetry_Init();
bool Telemetry_GetData(TelemetryData& data);
void Telemetry_UpdateData(const TelemetryData& data);

#endif
