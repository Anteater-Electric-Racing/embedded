// Anteater Electric Racing, 2025

#include "telemetry.h"

TelemetryData telemetryData;

void Telemetry_Init() {
    // fill with reasonable default values
    telemetryData = {0,0,0,0,0}; // current speed0 // starting speed
}

TelemetryData* Telemetry_GetData() {
    return &telemetryData;
}

void Telemetry_UpdateData(TelemetryData* data) {
    telemetryData = *data;
}
