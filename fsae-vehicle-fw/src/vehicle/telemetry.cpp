// Anteater Electric Racing, 2025

#include "telemetry.h"

TelemetryData telemetryData;

void Telemetry_Init() {
    // fill with reasonable default values
    telemetryData.APPS_Travel = 0;
    telemetryData.BSEFront_PSI = 0;
    telemetryData.BSERear_PSI = 0;

    telemetryData.accumulatorVoltage = 0;
    telemetryData.accumulatorTemp_F = 0;
    telemetryData.tractiveSystemVoltage = 0;
}

TelemetryData const* Telemetry_GetData() {
    return &telemetryData;
}

void Telemetry_UpdateData(TelemetryData* data) {
    telemetryData = *data;
}
