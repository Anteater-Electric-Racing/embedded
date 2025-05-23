// Anteater Electric Racing, 2025

#define THREAD_CAN_STACK_SIZE 128
#define THREAD_CAN_PRIORITY 1

#include "telemetry.h"
#include "utils/circular_buf.h"
#include <arduino_freertos.h>

#include "vehicle/apps.h"
#include "vehicle/bse.h"
#include "vehicle/faults.h"
#include "vehicle/motor.h"
#include "vehicle/telemetry.h"

TelemetryData telemetryData;
static CircularBuffer<TelemetryData> telemetryBuffer(10);

void Telemetry_Init() {
    // fill with reasonable default values
    telemetryData = {.APPS_Travel = 0,
                     .BSEFront_PSI = 0,
                     .BSERear_PSI = 0,

                     .accumulatorVoltage = 0,
                     .accumulatorTemp_F = 0,
                     .tractiveSystemVoltage = 0};

    xTaskCreate(threadTelemetry, "threadTelemetry", THREAD_CAN_STACK_SIZE, NULL,
                THREAD_CAN_PRIORITY, NULL);
}

void threadTelemetry(void *pvParameters) {
    while (true) {
        // mutex lock
        telemetryData.APPS_Travel = APPS_GetAPPSReading();
        telemetryData.BSEFront_PSI = BSE_GetBSEReading()->bseFront_PSI;
        telemetryData.BSERear_PSI = BSE_GetBSEReading()->bseRear_PSI;
        telemetryData.motorState = Motor_GetState();

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

bool Telemetry_GetData(TelemetryData &data) {
    return telemetryBuffer.get(data);
}

TelemetryData const *Telemetry_GetData() { return &telemetryData; }

void Telemetry_UpdateData(const TelemetryData &data) {
    telemetryBuffer.put(data);
}

void printTelemetryData(const TelemetryData &data) {
    Serial.print("Accumulator Voltage: ");
    Serial.println(data.accumulatorVoltage);
    Serial.print("Accumulator Temperature: ");
    Serial.println(data.accumulatorTemp);
    Serial.print("Tractive System Voltage: ");
    Serial.println(data.tractiveSystemVoltage);
}
