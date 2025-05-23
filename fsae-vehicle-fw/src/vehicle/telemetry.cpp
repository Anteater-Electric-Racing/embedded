// Anteater Electric Racing, 2025

#include "telemetry.h"
#include "utils/circular_buf.h"

TelemetryData telemetryData;
static CircularBuffer<TelemetryData> telemetryBuffer(10);

void Telemetry_Init() {
    // fill with reasonable default values
    telemetryData = {
        .accumulatorVoltage = 0,
        .accumulatorTemp = 0,
        .tractiveSystemVoltage = 0
    };
}

bool Telemetry_GetData(TelemetryData& data) {
    return telemetryBuffer.get(data);
}

void Telemetry_UpdateData(const TelemetryData& data) {
    telemetryBuffer.put(data);
}