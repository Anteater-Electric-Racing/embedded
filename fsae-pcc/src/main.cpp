// Anteater Electric Racing, 2025

#define THREAD_MAIN_STACK_SIZE 128
#define THREAD_MAIN_PRIORITY 1

#define SHUTDOWN_CIRCUIT_STACK_SIZE 512
#define SHUTDOWN_CIRCUIT_PRIORITY 2

#include <Arduino.h>
#include <arduino_freertos.h>
#include "precharge.cpp"
#include "states.h"

void threadMain(void *pvParameters);
void prechargeTask(void *pvParameters);
void monitorShutdownCircuitTask(void *pvParamters);

void setup() {
    xTaskCreate(threadMain, "threadMain", THREAD_MAIN_STACK_SIZE, NULL,
                THREAD_MAIN_PRIORITY, NULL);

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
