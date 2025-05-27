// Anteater Electric Racing, 2025

#include "telemetry.h"
#include <FlexCAN_T4.h>
#include <isotp.h>
#include <arduino_freertos.h>
#include <semphr.h>

SemaphoreHandle_t xSemaphore = xSemaphoreCreateMutex();

// TODO: Update IDs to be more logical values
const uint32_t canid = 0x666;
const uint32_t request = 0x777;
TelemetryData telemetryDataCAN;
isotp<RX_BANKS_16, 512> tp;
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> can2; // TODO: Figure out actual values for send and recieve

void threadTelemetryCAN( void *pvParameters );

void Telemetry_Init() {
    // TODO: Update initialization
    telemetryDataCAN = {
        .accumulatorVoltage = 0,
        .accumulatorTemp_F = 0,
        .tractiveSystemVoltage = 0
    };

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

void threadTelemetryCAN(void *pvParameters){
    uint8_t serializedTelemetryBuf[sizeof(TelemetryData)];
    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = 1;
    while(true){
        vTaskDelayUntil(&lastWakeTime, xFrequency);
        Serial.print("Semaphor take successful");
        xSemaphoreTake(xSemaphore, (TickType_t) 1000);
        Telemetry_SerializeData(telemetryDataCAN, serializedTelemetryBuf);
        xSemaphoreGive(xSemaphore);

        ISOTP_data config;
        config.id = 0x666;
        config.flags.extended = 0; /* standard frame */
        config.separation_time = 10; /* time between back-to-back frames in millisec */
        tp.write(config, serializedTelemetryBuf, sizeof(serializedTelemetryBuf));
    }
}

void Telemetry_SerializeData(TelemetryData data, uint8_t* serializedTelemetryBuf){
    memcpy(serializedTelemetryBuf, &data, sizeof(data));
}

TelemetryData const* Telemetry_GetData() {
    return &telemetryDataCAN;
}

void Telemetry_UpdateData(TelemetryData* data) {
    telemetryDataCAN = *data;
}

void Telemetry_UpdateADCData(volatile uint16_t* adc0reads, volatile uint16_t* adc1reads){
    xSemaphoreTake(xSemaphore, (TickType_t) 1000);
    memcpy(telemetryDataCAN.adc0Reads, (const uint16_t*)adc0reads, sizeof(telemetryDataCAN.adc0Reads));
    memcpy(telemetryDataCAN.adc1Reads, (const uint16_t*)adc1reads, sizeof(telemetryDataCAN.adc1Reads));
    xSemaphoreGive(xSemaphore);
}
