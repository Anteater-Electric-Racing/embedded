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
#include "can.h"

static void threadMain(void *pvParameters);

void setup() {
    Serial.begin(9600);

    xTaskCreate(threadMain, "threadMain", THREAD_MAIN_STACK_SIZE, NULL, THREAD_MAIN_PRIORITY, NULL);

    gpioInit(); // Initialize GPIO pins

    CAN_Init(); // Initialize CAN bus

    prechargeInit(); // Initialize precharge system

    vTaskStartScheduler();
}

void threadMain(void *pvParameters) {
    float accumulator_voltage = 0.0F;
    float ts_voltage = 0.0F;
    while (true) {
        accumulator_voltage = getAccumulatorVoltage();
        ts_voltage = getTSVoltage();
        Serial.print("Accumulator Voltage: " + String(accumulator_voltage) + "V");
        Serial.print(" | TS Voltage: ");
        Serial.print(ts_voltage, 4);
        Serial.print("V");
        Serial.print("\n");
        vTaskDelay(100);
    }
}

void loop() {
}
