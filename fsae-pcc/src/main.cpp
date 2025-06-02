// Anteater Electric Racing, 2025

#define THREAD_MAIN_STACK_SIZE 128
#define THREAD_MAIN_PRIORITY 1

#define PRECHARGE_PRIORITY 3
#define PRECHARGE_STACK_SIZE 512

#define SHUTDOWN_CIRCUIT_PRIORITY 2
#define SHUTDOWN_CIRCUIT_STACK_SIZE 512

// monitorShutdownCircuitTask defines
#define ACCUMULATOR_VOLTAGE_PIN 14
#define TS_VOLTAGE_PIN 15
#define PCC_RATIO .9

#include <Arduino.h>
#include <arduino_freertos.h>
#include "precharge.h"
#include "states.h"

static float alpha;
static float filteredFreq = 0.0F;

void threadMain(void *pvParameters);
void prechargeTask(void *pvParameters);

float getFrequency(int pin){
    const unsigned int TIMEOUT = 10000;
    unsigned int tHigh = pulseIn(pin, 1, TIMEOUT);  // microseconds
    unsigned int tLow = pulseIn(pin, 0, TIMEOUT);
    if (tHigh == 0 || tLow == 0){
        return 0; // timed out
    }
    return ( 1000000.0 / (float)(tHigh + tLow) );    // f = 1/T
}

float getVoltage(int pin){
    float voltage, rawFreq = getFrequency(pin);

    LOWPASS_FILTER(rawFreq, filteredFreq, alpha);

    Serial.print("frequency: ");
    Serial.print(filteredFreq);
    Serial.print("\r");
    // still need to convert freq to voltage here
    return voltage;
}

void monitorShutdownCircuitTask(void *pvParamters){
    float accumulator_voltage, ts_voltage;
    while (true) {
        accumulator_voltage = getVoltage(ACCUMULATOR_VOLTAGE_PIN);
        // ts_voltage = getVoltage(TS_VOLTAGE_PIN);
        // Serial.println("Accumulator Voltage: " + String(accumulator_voltage) + "V");
        // Serial.println("TS Voltage: " + String(ts_voltage) + "V");
        vTaskDelay(pdMS_TO_TICKS(1)); // Delay for 100ms before next reading
    }
}

void setup() {
    Serial.begin(9600);
    delay(2000);

    alpha = COMPUTE_ALPHA(1.0F); // 10Hz cutoff frequency for lowpass filter

    xTaskCreate(threadMain, "threadMain", THREAD_MAIN_STACK_SIZE, NULL,
                THREAD_MAIN_PRIORITY, NULL);

    // prechargeInit(); // Initialize precharge system

    xTaskCreate(monitorShutdownCircuitTask,
                "MonitorShutdownCircuitTask",
                SHUTDOWN_CIRCUIT_STACK_SIZE,
                NULL,
                SHUTDOWN_CIRCUIT_PRIORITY,
                NULL);

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
