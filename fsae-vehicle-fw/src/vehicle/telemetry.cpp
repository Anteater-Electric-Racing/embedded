// Anteater Electric Racing, 2025

#include "telemetry.h"

TelemetryData telemetryData;

void Telemetry_Init() {
    // fill with reasonable default values
    telemetryData = {
        .APPS_Travel = 0,
        .BSEFront_PSI = 0,
        .BSERear_PSI = 0,

        .accumulatorVoltage = 0,
        .accumulatorTemp_F = 0,
        .tractiveSystemVoltage = 0
    };
}

TelemetryData const* Telemetry_GetData() {
    return &telemetryData;
}

void Telemetry_UpdateData(TelemetryData* data) {
    telemetryData = *data;
}
