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
#include "gpio.h"
#include "utils.h"

static void threadMain(void *pvParameters);
static void prechargeTask(void *pvParameters);

void setup() {
    Serial.begin(9600);

    xTaskCreate(threadMain, "threadMain", THREAD_MAIN_STACK_SIZE, NULL, THREAD_MAIN_PRIORITY, NULL);

    gpioInit(); // Initialize GPIO pins

    prechargeInit(); // Initialize precharge system

    vTaskStartScheduler();
}

void threadMain(void *pvParameters) {
    float accumulator_voltage = 0.0F;
    float ts_voltage = 0.0F;
    while (true) {
        accumulator_voltage = getVoltage(ACCUMULATOR_VOLTAGE_PIN);
        ts_voltage = getVoltage(TS_VOLTAGE_PIN);
        Serial.print("Accumulator Voltage: " + String(accumulator_voltage) + "V");
        Serial.print(" | TS Voltage: " + String(ts_voltage) + "V | ");
        Serial.print(FREQ_TO_VOLTAGE(ts_voltage));
        Serial.print("\r");
        vTaskDelay(100);
    }
}

void loop() {
}
