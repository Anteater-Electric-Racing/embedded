// Anteater Electric Racing, 2025

#define THREAD_CAN_STACK_SIZE 128
#define THREAD_CAN_PRIORITY 1
#define THREAD_CAN_TELEMETRY_PRIORITY 2

#include "can.h"
#include <FlexCAN_T4.h>
#include <isotp.h>
#include <arduino_freertos.h>

FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_16> can3;
CAN_message_t msg;
CAN_message_t rx_msg;

// TODO: Update IDs to be more logical values
const uint32_t canid = 0x666;
const uint32_t request = 0x777;
CANTelemetryData telemetryDataCAN;
static uint8_t serializedTelemetryBuf[sizeof(CANTelemetryData)];
isotp<RX_BANKS_16, 512> tp;
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> can2; // TODO: Figure out actual values for send and recieve

void threadCAN( void *pvParameters );
void threadTelemetryCAN( void *pvParameters );

void CAN_TelemetryInit() {
    // fill with reasonable default values
    telemetryDataCAN = {
        .accumulatorVoltage = 0,
        .accumulatorTemp = 0,
        .tractiveSystemVoltage = 0
    };
    // TODO: Initialize adc reads data with more helpful default values
    uint16_t adc0Reads[SENSOR_PIN_AMT_ADC0] = {0, 0, 0, 0, 0, 0, 0, 0};
    uint16_t adc1Reads[SENSOR_PIN_AMT_ADC1] = {0, 0, 0, 0, 0, 0, 0, 0};
    CAN_SerializeTelemetryData(telemetryDataCAN); // May be problematic if we forget to call this somewhere after updating data
}

void CAN_Init() {
    // Initialize CAN bus
    can3.begin();
    can3.setBaudRate(500000);
    can3.setTX(DEF);
    can3.setRX(DEF);
    can3.enableFIFO();
    can3.enableFIFOInterrupt();
    can3.enableMBInterrupts();
    can3.setMaxMB(16);

    CAN_TelemetryInit();
    can2.begin();
    can2.setBaudRate(1000000);
    can2.setTX(DEF);
    can2.setRX(DEF);
    can2.enableFIFO();
    can2.enableFIFOInterrupt();
    can2.setMaxMB(16);
    tp.begin();
    tp.setWriteBus(&can2); /* we write to this bus */
}

void CAN_Begin() {
    xTaskCreate(threadCAN, "threadCAN", THREAD_CAN_STACK_SIZE, NULL, THREAD_CAN_PRIORITY, NULL);
    xTaskCreate(threadTelemetryCAN, "threadTelemetryCAN", THREAD_CAN_STACK_SIZE, NULL, THREAD_CAN_TELEMETRY_PRIORITY, NULL);

}

void threadCAN(void *pvParameters){
    while(true){
        msg.id = 0x123; // Set the CAN ID
        msg.len = 8; // Set the length of the message
        // int res = can3.write(MB0, msg)
        int res = can3.write(msg);
        // can3.mailboxStatus();
        Serial.println(res);
        msg.id++;
        Serial.println("CAN thread running...");
        vTaskDelay(100);
    }
}

void threadTelemetryCAN(void *pvParameters){
    while(true){
        static uint32_t sendTimer = millis();
        if ( millis() - sendTimer > 1000 ) {
            uint8_t buf[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5 };
            const char b[] = "01413AAAAABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
            ISOTP_data config;
            config.id = 0x666;
            config.flags.extended = 0; /* standard frame */
            config.separation_time = 10; /* time between back-to-back frames in millisec */
            tp.write(config, buf, sizeof(buf));
            tp.write(config, b, sizeof(b));
            sendTimer = millis();
        }
    }

}

void floatToBytes(float val, uint8_t* buf) {
    memcpy(buf, &val, sizeof(float));
}

void CAN_SerializeTelemetryData(CANTelemetryData data){
    floatToBytes(data.accumulatorVoltage, serializedTelemetryBuf); // copy float accumulator voltage (bytes 0-3)
    floatToBytes(data.accumulatorTemp, serializedTelemetryBuf + 4); // copy float accumulator temp (bytes 4-7)
    floatToBytes(data.tractiveSystemVoltage, serializedTelemetryBuf + 8); // copy float tractive system voltage (bytes 8-11)
    memcpy(serializedTelemetryBuf + 12, &data, sizeof(data) - 12); // copy rest of data
}

void CAN_UpdateTelemetryData(CANTelemetryData* data) {
    telemetryDataCAN = *data;
}

void CAN_UpdateTelemetryADCData(volatile uint16_t* adc0reads, volatile uint16_t* adc1reads){
    memcpy(telemetryDataCAN.adc0Reads, (const uint16_t*)adc0reads, sizeof(telemetryDataCAN.adc0Reads));
    memcpy(telemetryDataCAN.adc1Reads, (const uint16_t*)adc1reads, sizeof(telemetryDataCAN.adc1Reads));
    CAN_SerializeTelemetryData(telemetryDataCAN);
}
