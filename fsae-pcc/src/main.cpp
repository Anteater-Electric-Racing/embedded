// Anteater Electric Racing, 2025

#define THREAD_MAIN_STACK_SIZE 128
#define THREAD_MAIN_PRIORITY 1

#define PRECHARGE_PRIORITY 3
#define PRECHARGE_STACK_SIZE 512

#define SHUTDOWN_CIRCUIT_PRIORITY 2
#define SHUTDOWN_CIRCUIT_STACK_SIZE 512

// monitorShutdownCircuitTask defines
#define ACCUMULATOR_VOLTAGE_PIN 21 
#define TS_VOLTAGE_PIN 22
#define PCC_RATIO .9


#include <Arduino.h>
#include <arduino_freertos.h>
#include "precharge.cpp"
#include "states.h"

void threadMain(void *pvParameters);
void prechargeTask(void *pvParameters);
void monitorShutdownCircuitTask(void *pvParamters){
    float accumulator_voltage, ts_voltage;
    do {
        accumulator_voltage = getVoltage(ACCUMULATOR_VOLTAGE_PIN);
        ts_voltage = getVoltage(TS_VOLTAGE_PIN);
    } 
    while(ts_voltage < accumulator_voltage * PCC_RATIO);
    // update variable containing voltage state
}

float getVoltage(int pin){
    float voltage, freq = getFrequency(pin);
    // still need to convert freq to voltage here
    return voltage;
}

float getFrequency(int pin){
    const unsigned int TIMEOUT = 10000;
    unsigned int tHigh = pulseIn(pin, 1, TIMEOUT);  // microseconds
    unsigned int tLow = pulseIn(pin, 0, TIMEOUT);
    if (tHigh == 0 || tLow == 0){
        return 0; // timed out
    }
    return ( 1000000.0 / (float)(tHigh + tLow) );    // f = 1/T
}

void setup() {
    xTaskCreate(threadMain, "threadMain", THREAD_MAIN_STACK_SIZE, NULL,
                THREAD_MAIN_PRIORITY, NULL);

    xTaskCreate(prechargeTask,          // Task function
                "PrechargeTask",        // Task name
                PRECHARGE_STACK_SIZE,   // Stack size
                NULL,                   // Parameters
                PRECHARGE_PRIORITY,     // Priority
                NULL);                  // Task handle

    xTaskCreate(monitorShutdownCircuitTask, 
                "MonitorShutdownCircuitTask", 
                SHUTDOWN_CIRCUIT_STACK_SIZE, 
                NULL,
                SHUTDOWN_CIRCUIT_PRIORITY, 
                NULL);
    
    vTaskStartScheduler();
}

void threadMain(void *pvParameters) {
}

void loop() {}
