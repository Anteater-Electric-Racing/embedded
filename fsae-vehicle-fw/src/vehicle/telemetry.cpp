// Anteater Electric Racing, 2025
#define THREAD_CAN_TELEMETRY_STACK_SIZE 128
#define THREAD_CAN_TELEMETRY_PRIORITY 2

#include "telemetry.h"
#include <isotp.h>
#include <arduino_freertos.h>
#include <semphr.h>
#include "peripherals/can.h"

SemaphoreHandle_t xSemaphore = xSemaphoreCreateMutex();

// TODO: Update IDs to be more logical values
const uint32_t canid = 0x666;
const uint32_t request = 0x777;
TelemetryData telemetryDataCAN;
isotp<RX_BANKS_16, 512> tp;

void threadTelemetryCAN( void *pvParameters );

void Telemetry_Init() {
    // TODO: Update initialization
    telemetryDataCAN = {
        .accumulatorVoltage = 0,
        .accumulatorTemp_F = 0,
        .tractiveSystemVoltage = 0
    };

    tp.begin();
    // tp.setWriteBus(&can2); /* we write to this bus */

}

void Telemetry_Begin() {
    xTaskCreate(threadTelemetryCAN, "threadTelemetryCAN", THREAD_CAN_TELEMETRY_STACK_SIZE, NULL, THREAD_CAN_TELEMETRY_PRIORITY, NULL);
}

void threadTelemetryCAN(void *pvParameters){
    uint8_t serializedTelemetryBuf[sizeof(TelemetryData)];
    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = 1;
    while(true){
        // vTaskDelayUntil(&lastWakeTime, xFrequency);
        // Serial.print("Semaphor take successful");
        // xSemaphoreTake(xSemaphore, (TickType_t) 1000);
        // Telemetry_SerializeData(telemetryDataCAN, serializedTelemetryBuf);
        // xSemaphoreGive(xSemaphore);

        // ISOTP_data config;
        // config.id = 0x666;
        // config.flags.extended = 0; /* standard frame */
        // config.separation_time = 10; /* time between back-to-back frames in millisec */
        // tp.write(config, serializedTelemetryBuf, sizeof(serializedTelemetryBuf));
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
