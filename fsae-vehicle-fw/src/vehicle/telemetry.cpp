// // Anteater Electric Racing, 2025

// #include "telemetry.h"
// // #include <FlexCAN_T4.h>
// #include <isotp.h>
// #include <arduino_freertos.h>

// #define THREAD_CAN_STACK_SIZE 128
// #define THREAD_CAN_PRIORITY 2


// const uint32_t canid = 0x666; // TODO: Get real CAN ID
// const uint32_t request = 0x777; // TODO: Get real request ID
// TelemetryData telemetryData;
// static uint8_t serializedTelemetryBuf[sizeof(TelemetryData)];
// // isoisoTP_server<canid, STANDARD_ID, request, serializedTelemetryBuf, sizeof(serializedTelemetryBuf)> myResource;
// isotp<RX_BANKS_16, 512> isoTP;

// FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> can2Telemetry; // TODO: Figure out actual values for send and recieve

// void Telemetry_Init() {
//     // fill with reasonable default values
//     telemetryData = {
//         .accumulatorVoltage = 0,
//         .accumulatorTemp = 0,
//         .tractiveSystemVoltage = 0
//     };
//     // TODO: Initialize adc reads data with more helpful default values
//     uint16_t adc0Reads[SENSOR_PIN_AMT_ADC0] = {0, 0, 0, 0, 0, 0, 0, 0};
//     uint16_t adc1Reads[SENSOR_PIN_AMT_ADC1] = {0, 0, 0, 0, 0, 0, 0, 0};
//     Telemetry_SerializeData(telemetryData); // May be problematic if we forget to call this somewhere after updating data

//     Telemetry_CANSetup();
// }

// void Telemetry_CanSniff(const CAN_message_t &msg) {
//   Serial.print("MB "); Serial.print(msg.mb);
//   Serial.print(" OVERRUN: "); Serial.print(msg.flags.overrun);
//   Serial.print(" LEN: "); Serial.print(msg.len);
//   Serial.print(" EXT: "); Serial.print(msg.flags.extended);
//   Serial.print(" TS: "); Serial.print(msg.timestamp);
//   Serial.print(" ID: "); Serial.print(msg.id, arduino::HEX);
//   Serial.print(" BUS: "); Serial.print(msg.bus);
//   Serial.print(" Buffer: ");
//   for ( uint8_t i = 0; i < msg.len; i++ ) {
//     Serial.print(msg.buf[i], arduino::HEX); Serial.print(" ");
//   } Serial.println();
// }

// void Telemetry_CANSetup() {
//     // TODO: ensure these numbers line up with expected
//     Serial.begin(115200);
//     vTaskDelay(1000); // Delay for 1 second
//     can2Telemetry.begin();
//     // can2Telemetry.setClock(CLK_60MHz);
//     can2Telemetry.setBaudRate(1000000);
//     can2Telemetry.setTX(DEF);
//     can2Telemetry.setRX(DEF);
//     can2Telemetry.enableFIFO();
//     can2Telemetry.enableFIFOInterrupt();
//     can2Telemetry.setMaxMB(16);
//     isoTP.begin();
//     isoTP.setWriteBus(&can2Telemetry); /* we write to this bus */
//     can2Telemetry.onReceive(Telemetry_CanSniff);
// }

// void telemetryCanThread( void *pvParameters );

// void Telemetry_Begin() {
//     xTaskCreate(telemetryCanThread, "telemetryCanThread", THREAD_CAN_STACK_SIZE, NULL, THREAD_CAN_PRIORITY, NULL);
// }

// void telemetryCanThread(void *pvParameters){
//     while(true){
//         static uint32_t sendTimer = millis();
//         if ( millis() - sendTimer > 1000 ) {
//             uint8_t buf[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5 };
//             const char b[] = "01413AAAAABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
//             ISOTP_data config;
//             config.id = 0x666;
//             config.flags.extended = 0; /* standard frame */
//             config.separation_time = 10; /* time between back-to-back frames in millisec */
//             isoTP.write(config, buf, sizeof(buf));
//             isoTP.write(config, b, sizeof(b));
//             sendTimer = millis();
//         }
//     }

// }

// TelemetryData* Telemetry_GetData() {
//     return &telemetryData;
// }

// void floatToBytesTelemetry(float val, uint8_t* buf) {
//     memcpy(buf, &val, sizeof(float));
// }

// void Telemetry_SerializeData(TelemetryData data){
//     floatToBytesTelemetry(data.accumulatorVoltage, serializedTelemetryBuf); // copy float accumulator voltage (bytes 0-3)
//     floatToBytesTelemetry(data.accumulatorTemp, serializedTelemetryBuf + 4); // copy float accumulator temp (bytes 4-7)
//     floatToBytesTelemetry(data.tractiveSystemVoltage, serializedTelemetryBuf + 8); // copy float tractive system voltage (bytes 8-11)
//     memcpy(serializedTelemetryBuf + 12, &data, sizeof(data) - 12); // copy rest of data
// }


// void Telemetry_UpdateData(TelemetryData* data) {
//     telemetryData = *data;
// }

// void Telemetry_UpdateADCData(volatile uint16_t* adc0reads, volatile uint16_t* adc1reads){
//     memcpy(telemetryData.adc0Reads, (const uint16_t*)adc0reads, sizeof(telemetryData.adc0Reads));
//     memcpy(telemetryData.adc1Reads, (const uint16_t*)adc1reads, sizeof(telemetryData.adc1Reads));
//     Telemetry_SerializeData(telemetryData);
// }



// // void Telemetry_TransmitData(){
// //     Telemetry_SerializeData(telemetryData);
// //     // Figure out how to actually transmit it
// // }
