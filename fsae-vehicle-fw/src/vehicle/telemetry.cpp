// Anteater Electric Racing, 2025

#include "telemetry.h"

TelemetryData telemetryData;

void Telemetry_Init() {
    // fill with reasonable default values
    telemetryData = {
        .accumulatorVoltage = 0,
        .accumulatorTemp = 0,
        .tractiveSystemVoltage = 0
    };
}

TelemetryData* Telemetry_GetData() {
    return &telemetryData;
}
