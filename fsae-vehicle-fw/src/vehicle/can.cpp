// Anteater Electric Racing, 2025

#include <FlexCAN_T4.h>
#include <isotp.h>

#include <arduino_freertos.h>
#include <semphr.h>

#include "can.h"
#include "peripherals/adc.h"

#define THREAD_CAN_STACK_SIZE 128
#define THREAD_CAN_PRIORITY 1
#define THREAD_CAN_TELEMETRY_PRIORITY 2
#define QUEUE_TELEMETRY_ADC_LENGTH 3
#define QUEUE_TELEMETRY_ADC_ITEM_SIZE                                          \
    (sizeof(uint16_t) *                                                        \
     (SENSOR_PIN_AMT_ADC0 +                                                    \
      SENSOR_PIN_AMT_ADC1)) // TODO: figure out how big to make this

FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_16> can3;
CAN_message_t msg;
CAN_message_t rx_msg;

SemaphoreHandle_t xSemaphore;
QueueHandle_t adcValuesQueue;

// TODO: Update IDs to be more logical values
const uint32_t canid = 0x666;
const uint32_t request = 0x777;
CANTelemetryData telemetryDataCAN;
isotp<RX_BANKS_16, 512> tp;
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16>
    can2; // TODO: Figure out actual values for send and recieve

void threadCAN(void *pvParameters);
void threadTelemetryCAN(void *pvParameters);

void CAN_TelemetryInit() {
    xSemaphore = xSemaphoreCreateMutex();
    // fill with reasonable default values
    telemetryDataCAN = {.accumulatorVoltage = 0,
                        .accumulatorTemp = 0,
                        .tractiveSystemVoltage = 0};
    // TODO: Initialize adc reads data with more helpful default values
    uint16_t adc0Reads[SENSOR_PIN_AMT_ADC0] = {0, 0, 0, 0, 0, 0, 0, 0};
    uint16_t adc1Reads[SENSOR_PIN_AMT_ADC1] = {0, 0, 0, 0, 0, 0, 0, 0};
    // CAN_SerializeTelemetryData(telemetryDataCAN, ); // May be problematic if
    // we forget to call this somewhere after updating data
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
    // xTaskCreate(threadCAN, "threadCAN", THREAD_CAN_STACK_SIZE, NULL,
    // THREAD_CAN_PRIORITY, NULL);
    xTaskCreate(threadTelemetryCAN, "threadTelemetryCAN", THREAD_CAN_STACK_SIZE,
                NULL, THREAD_CAN_TELEMETRY_PRIORITY, NULL);
    adcValuesQueue =
        xQueueCreate(QUEUE_TELEMETRY_ADC_LENGTH, QUEUE_TELEMETRY_ADC_ITEM_SIZE);
}

void threadCAN(void *pvParameters) {
    while (true) {
        msg.id = 0x123; // Set the CAN ID
        msg.len = 8;    // Set the length of the message
        // int res = can3.write(MB0, msg)
        int res = can3.write(msg);
        // can3.mailboxStatus();
        Serial.println(res);
        msg.id++;
        Serial.println("CAN thread running...");
        vTaskDelay(100);
    }
}

void threadTelemetryCAN(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = 1000;
    uint8_t serializedTelemetryBuf[sizeof(CANTelemetryData)];
    CANADCValues newValues;
    while (true) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        xSemaphoreTake(xSemaphore, (TickType_t)10);
        while (uxQueueMessagesWaiting(adcValuesQueue)) {
            xQueueReceive(adcValuesQueue, &newValues, portMAX_DELAY);
            memcpy(telemetryDataCAN.adc0Reads, newValues.adc0Reads,
                   sizeof(newValues.adc0Reads));
            memcpy(telemetryDataCAN.adc1Reads, newValues.adc1Reads,
                   sizeof(newValues.adc1Reads));
            CAN_SerializeTelemetryData(telemetryDataCAN,
                                       serializedTelemetryBuf);
        }
        xSemaphoreGive(xSemaphore);

        ISOTP_data config;
        config.id = 0x666;
        config.flags.extended = 0; /* standard frame */
        config.separation_time =
            10; /* time between back-to-back frames in millisec */
        tp.write(config, serializedTelemetryBuf,
                 sizeof(serializedTelemetryBuf));
    }
}

void floatToBytes(float val, uint8_t *buf) { memcpy(buf, &val, sizeof(float)); }

void CAN_SerializeTelemetryData(CANTelemetryData data,
                                uint8_t *serializedTelemetryBuf) {
    memcpy(serializedTelemetryBuf, &data, sizeof(data));
}

void CAN_UpdateTelemetryData(CANTelemetryData *data) {
    telemetryDataCAN = *data;
}

void CAN_UpdateAccumulatorData(float accumulatorVoltage, float accumulatorTemp,
                               float tractiveSystemVoltage) {
    // xSemaphoreTake(xSemaphore, (TickType_t) 1000);
    telemetryDataCAN.accumulatorVoltage = accumulatorVoltage;
    telemetryDataCAN.accumulatorTemp = accumulatorTemp;
    telemetryDataCAN.tractiveSystemVoltage = tractiveSystemVoltage;
    // xSemaphoreGive(xSemaphore);
}

void CAN_UpdateTelemetryADCData(volatile uint16_t *adc0reads,
                                volatile uint16_t *adc1reads) {
    BaseType_t xHigherPriorityTaskWoken =
        pdFALSE; // TODO: check if this is correct
    CANADCValues newValues = {.adc0Reads = *adc0reads, .adc1Reads = *adc1reads};
    xQueueSendToBackFromISR(adcValuesQueue, &newValues,
                            &xHigherPriorityTaskWoken);

    // xSemaphoreTake(xSemaphore, (TickType_t) 1000);
    // memcpy(telemetryDataCAN.adc0Reads, (const uint16_t*)adc0reads,
    // sizeof(telemetryDataCAN.adc0Reads)); memcpy(telemetryDataCAN.adc1Reads,
    // (const uint16_t*)adc1reads, sizeof(telemetryDataCAN.adc1Reads));
    // xSemaphoreGive(xSemaphore);
}
