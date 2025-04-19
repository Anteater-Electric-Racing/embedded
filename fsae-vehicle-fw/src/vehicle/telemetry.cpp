// Anteater Electric Racing, 2025

#include "telemetry.h"
#include <FlexCAN_T4.h>
#include <isotp_server.h>

const uint32_t canid = 0x666; // TODO: Get real CAN ID
const uint32_t request = 0x777; // TODO: Get real request ID
TelemetryData telemetryData;
static uint8_t serializedTelemetryBuf[sizeof(TelemetryData)];
isotp_server<canid, STANDARD_ID, request, serializedTelemetryBuf, sizeof(serializedTelemetryBuf)> myResource;

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can1; // TODO: Figure out actual values for send and recieve

void Telemetry_Init() {
    // fill with reasonable default values
    telemetryData = {
        .accumulatorVoltage = 0,
        .accumulatorTemp = 0,
        .tractiveSystemVoltage = 0
    };
    // TODO: Initialize adc reads data with more helpful default values
    uint16_t adc0Reads[SENSOR_PIN_AMT_ADC0] = {0, 0, 0, 0, 0, 0, 0, 0};
    uint16_t adc1Reads[SENSOR_PIN_AMT_ADC1] = {0, 0, 0, 0, 0, 0, 0, 0};
    Telemetry_SerializeData(telemetryData); // May be problematic if we forget to call this somewhere after updating data

    Telemetry_CANSetup();
}

void Telemetry_CanSniff(const CAN_message_t &msg) {
  Serial.print("MB "); Serial.print(msg.mb);
  Serial.print(" OVERRUN: "); Serial.print(msg.flags.overrun);
  Serial.print(" LEN: "); Serial.print(msg.len);
  Serial.print(" EXT: "); Serial.print(msg.flags.extended);
  Serial.print(" TS: "); Serial.print(msg.timestamp);
  Serial.print(" ID: "); Serial.print(msg.id, HEX);
  Serial.print(" BUS: "); Serial.print(msg.bus);
  Serial.print(" Buffer: ");
  for ( uint8_t i = 0; i < msg.len; i++ ) {
    Serial.print(msg.buf[i], HEX); Serial.print(" ");
  } Serial.println();
}

void Telemetry_CANSetup() {
    // TODO: ensure these numbers line up with expected
    Serial.begin(115200);
    delay(400);
    Can1.begin();
    Can1.setClock(CLK_60MHz);
    Can1.setBaudRate(95238);
    Can1.setMaxMB(16);
    Can1.enableFIFO();
    Can1.enableFIFOInterrupt();
    Can1.onReceive(Telemetry_CanSniff);
    myResource.begin();
    myResource.setWriteBus(&Can1); /* we write to this bus */
}

TelemetryData* Telemetry_GetData() {
    return &telemetryData;
}

void Telemetry_UpdateData(TelemetryData* data) {
    telemetryData = *data;
}

void Telemetry_UpdateADCData(volatile uint16_t* adc0reads, volatile uint16_t* adc1reads){
    memcpy(telemetryData.adc0Reads, (const uint16_t*)adc0reads, sizeof(telemetryData.adc0Reads));
    memcpy(telemetryData.adc1Reads, (const uint16_t*)adc1reads, sizeof(telemetryData.adc1Reads));
    Telemetry_SerializeData(telemetryData);
}

void floatToBytes(float val, uint8_t* buf) {
    memcpy(buf, &val, sizeof(float));
}

void Telemetry_SerializeData(TelemetryData data){
    floatToBytes(data.accumulatorVoltage, serializedTelemetryBuf); // copy float accumulator voltage (bytes 0-3)
    floatToBytes(data.accumulatorTemp, serializedTelemetryBuf + 4); // copy float accumulator temp (bytes 4-7)
    floatToBytes(data.tractiveSystemVoltage, serializedTelemetryBuf + 8); // copy float tractive system voltage (bytes 8-11)
    memcpy(serializedTelemetryBuf + 12, &data, sizeof(data) - 12); // copy rest of data
}

// void Telemetry_TransmitData(){
//     Telemetry_SerializeData(telemetryData);
//     // Figure out how to actually transmit it
// }
