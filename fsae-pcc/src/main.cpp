// Anteater Electric Racing, 2025

#define THREAD_MAIN_STACK_SIZE 128
#define THREAD_MAIN_PRIORITY 1

#define PRECHARGE_PRIORITY 3
#define PRECHARGE_STACK_SIZE 512

#define SHUTDOWN_CIRCUIT_PRIORITY 2
#define SHUTDOWN_CIRCUIT_STACK_SIZE 512

#include <Arduino.h>
#include <arduino_freertos.h>
#include "precharge.h"

void threadMain(void *pvParameters);
void prechargeTask(void *pvParameters);

void monitorShutdownCircuitTask(void *pvParamters){
    float accumulator_voltage, ts_voltage;
    while (true) {
        accumulator_voltage = getVoltage(ACCUMULATOR_VOLTAGE_PIN);
        ts_voltage = getVoltage(TS_VOLTAGE_PIN);
        Serial.println("Accumulator Voltage: " + String(accumulator_voltage) + "V");
        Serial.println("TS Voltage: " + String(ts_voltage) + "V");
        vTaskDelay(pdMS_TO_TICKS(1)); // Delay for 100ms before next reading
    }
}

void setup() {
    Serial.begin(9600);
    delay(2000);

    xTaskCreate(threadMain, "threadMain", THREAD_MAIN_STACK_SIZE, NULL, THREAD_MAIN_PRIORITY, NULL);

    prechargeInit(); // Initialize precharge system

    xTaskCreate(monitorShutdownCircuitTask, "MonitorShutdownCircuitTask", SHUTDOWN_CIRCUIT_STACK_SIZE, NULL, SHUTDOWN_CIRCUIT_PRIORITY, NULL);

    vTaskStartScheduler();
}

void threadMain(void *pvParameters) {
    while (true) {
        // Serial.println("Thead main running");
        // delay(1000); // Simulate some work
    }
}

void loop() {
}
