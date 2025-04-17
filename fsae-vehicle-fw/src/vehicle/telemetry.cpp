// Anteater Electric Racing, 2025

#include "telemetry.h"
#include <FlexCAN_T4.h>
#include <isotp_server.h>

TelemetryData telemetryData;
static uint8_t serializedTelemetryBuf[sizeof(TelemetryData)];

void Telemetry_Init() {
    // fill with reasonable default values
    telemetryData = {
        .accumulatorVoltage = 0,
        .accumulatorTemp = 0,
        .tractiveSystemVoltage = 0
    };
    // TODO: Initialize adc reads data
}

TelemetryData* Telemetry_GetData() {
    return &telemetryData;
}

void Telemetry_UpdateData(TelemetryData* data) {
    telemetryData = *data;
}

void floatToBytes(float val, uint8_t* buf) {
    memcpy(buf, &val, sizeof(float));
}


void Telemetry_SerializeData(TelemetryData data){
    floatToBytes(data.accumulatorVoltage, serializedTelemetryBuf); // copy float accumulator voltage (bytes 0-3)
    floatToBytes(data.accumulatorTemp, serializedTelemetryBuf + 4); // copy float accumulator temp (bytes 4-7)
    floatToBytes(data.tractiveSystemVoltage, serializedTelemetryBuf + 8); // copy float tractive system voltage (bytes 8-11)
    memcpy(serializedTelemetryBuf + 12, &data, sizeof(data)); // copy rest of data
}

void Telemetry_TransmitData(){
    uint8_t serializedDataBuf[sizeof(telemetryData)];
    Telemetry_SerializeData(telemetryData);
    const uint32_t canid = 0x666;
    const uint32_t request = 0x020902;

    isotp_server<canid, STANDARD_ID, request, serializedTelemetryBuf, sizeof(serializedTelemetryBuf)> myResource;

}
