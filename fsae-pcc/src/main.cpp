// Anteater Electric Racing, 2025

#define THREAD_MAIN_STACK_SIZE 512
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
    PrechargeState state = STATE_UNDEFINED;
    while (true) {
        digitalWrite(13, HIGH); // Blink the built-in LED
        // accumulator_voltage = getAccumulatorVoltage();
        // ts_voltage = getTSVoltage();
        accumulator_voltage = getAccumulatorVoltage();
        ts_voltage = getTSVoltage();
        state = getPrechargeState();

        Serial.print("State: ");
        switch (state) {
            case STATE_STANDBY:
                Serial.print("STANDBY");
                break;
            case STATE_PRECHARGE:
                Serial.print("PRECHARGE");
                break;
            case STATE_DISCHARGE:
                Serial.print("DISCHARGE");
                break;
            case STATE_ONLINE:
                Serial.print("ONLINE");
                break;
            case STATE_ERROR:
                Serial.print("ERROR");
                break;
            default:
                Serial.print("UNDEFINED");
                break;
        }
        Serial.print(" | Accumulator Voltage: ");
        Serial.print(accumulator_voltage, 4);
        Serial.print("V");
        Serial.print(" | TS Voltage: ");
        Serial.print(ts_voltage, 4);
        Serial.print("V");
        Serial.print("\r");
        vTaskDelay(100);
    }
}

void loop() {
}
