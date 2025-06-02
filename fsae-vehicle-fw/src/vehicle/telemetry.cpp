// Anteater Electric Racing, 2025

#include <isotp.h>
#include <arduino_freertos.h>
#include <semphr.h>

#include "vehicle/apps.h"
#include "vehicle/bse.h"
#include "vehicle/telemetry.h"
#include "peripherals/can.h"

#define THREAD_CAN_TELEMETRY_STACK_SIZE 128
#define THREAD_CAN_TELEMETRY_PRIORITY 2

SemaphoreHandle_t xSemaphore = xSemaphoreCreateMutex();

// TODO: Update IDs to be more logical values
const uint32_t canid = 0x666;
const uint32_t request = 0x777;
TelemetryData telemetryData;
isotp<RX_BANKS_16, 512> tp;
static TickType_t lastWakeTime;

static void threadTelemetry(void *pvParameters);

void Telemetry_Init() {
    // TODO: Update initialization
    telemetryData = { // fill with reasonable dummy values
        .APPS_Travel = 0.0F,
        .BSEFront_PSI = 0.0F,
        .BSERear_PSI = 0.0F,
        .accumulatorVoltage = 0.0F,
        .accumulatorTemp_F = 0.0F,
        .motorState = MOTOR_STATE_OFF,
        .motorSpeed = 0.0F,
        .motorTorque = 0.0F,
        .maxMotorTorque = 0.0F,
        .maxMotorBrakeTorque = 0.0F,
        .motorDirection = DIRECTION_STANDBY,
        .mcuMainState = STATE_STANDBY,
        .mcuWorkMode = WORK_MODE_STANDBY,
        .motorTemp = 25,
        .mcuTemp = 25,
        .dcMainWireOverVoltFault = false,
        .motorPhaseCurrFault = false,
        .mcuOverHotFault = false,
        .resolverFault = false,
        .phaseCurrSensorFault = false,
        .motorOverSpdFault = false,
        .drvMotorOverHotFault = false,
        .dcMainWireOverCurrFault = false,
        .drvMotorOverCoolFault = false,
        .mcuMotorSystemState = false,
        .mcuTempSensorState = false,
        .motorTempSensorState = false,
        .dcVoltSensorState = false,
        .dcLowVoltWarning = false,
        .mcu12VLowVoltWarning = false,
        .motorStallFault = false,
        .motorOpenPhaseFault = false,
        .mcuWarningLevel = ERROR_NONE,
        .mcuVoltage = 0.0F,
        .mcuCurrent = 0.0F,
        .motorPhaseCurr = 0.0F,
    };

    tp.begin();
    // tp.setWriteBus(&can2); /* we write to this bus */
}

void Telemetry_Begin() {
    xTaskCreate(threadTelemetry, "threadTelemetry", THREAD_CAN_TELEMETRY_STACK_SIZE, NULL, THREAD_CAN_TELEMETRY_PRIORITY, NULL);
}

void threadTelemetry(void *pvParameters){
    uint8_t serializedTelemetryBuf[sizeof(TelemetryData)];
    const TickType_t xFrequency = 1;
    while(true){
        lastWakeTime = xTaskGetTickCount(); // Initialize the last wake time

        taskENTER_CRITICAL(); // Enter critical section
        telemetryData = {
            .APPS_Travel = APPS_GetAPPSReading(),
            .BSEFront_PSI = BSE_GetBSEReading()->bseFront_PSI,
            .BSERear_PSI = BSE_GetBSEReading()->bseFront_PSI,
            .accumulatorVoltage = 0.0F, // TODO: Replace with actual accumulator voltage reading
            .accumulatorTemp_F = 0.0F, // TODO: Replace with actual accumulator temperature reading
            .motorState = Motor_GetState(),
        };
        taskEXIT_CRITICAL();
        // Serial.print("Semaphor take successful");
        // xSemaphoreTake(xSemaphore, (TickType_t) 1000);
        // Telemetry_SerializeData(telemetryDataCAN, serializedTelemetryBuf);
        // xSemaphoreGive(xSemaphore);

        // ISOTP_data config;
        // config.id = 0x666;
        // config.flags.extended = 0; /* standard frame */
        // config.separation_time = 10; /* time between back-to-back frames in millisec */
        // tp.write(config, serializedTelemetryBuf, sizeof(serializedTelemetryBuf));
        vTaskDelayUntil(&lastWakeTime, xFrequency);
    }
}

void Telemetry_SerializeData(TelemetryData data, uint8_t* serializedTelemetryBuf){
    memcpy(serializedTelemetryBuf, &data, sizeof(data));
}

TelemetryData const* Telemetry_GetData() {
    return &telemetryData;
}

// void Telemetry_UpdateADCData(volatile uint16_t* adc0reads, volatile uint16_t* adc1reads){
//     xSemaphoreTake(xSemaphore, (TickType_t) 1000);
//     memcpy(telemetryDataCAN.adc0Reads, (const uint16_t*)adc0reads, sizeof(telemetryDataCAN.adc0Reads));
//     memcpy(telemetryDataCAN.adc1Reads, (const uint16_t*)adc1reads, sizeof(telemetryDataCAN.adc1Reads));
//     xSemaphoreGive(xSemaphore);
// }
