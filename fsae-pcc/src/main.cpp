// Anteater Electric Racing, 2025

#define THREAD_MAIN_STACK_SIZE 128
#define THREAD_MAIN_PRIORITY 1

#define PRECHARGE_PRIORITY 4
#define PRECHARGE_STACK_SIZE 512

#define UPDATE_STATUS_PRIORITY 3
#define UPDATE_STATUS_STACK_SIZE 512

#define SHUTDOWN_CIRCUIT_PRIORITY 2
#define SHUTDOWN_CIRCUIT_STACK_SIZE 512


#include <Arduino.h>
#include <arduino_freertos.h>
#include "pcc.cpp"

void threadMain(void *pvParameters);
void prechargeTask(void *pvParameters);
void updateStatusTask(void *pvParameters);
void monitorShutdownCircuitTask(void *pvParamters);

void setup() { // runs once on bootup
    xTaskCreate(threadMain, "threadMain", THREAD_MAIN_STACK_SIZE, NULL,
                THREAD_MAIN_PRIORITY, NULL);

    xTaskCreate(prechargeTask,          // Task function
                "PrechargeTask",        // Task name
                PRECHARGE_STACK_SIZE,   // Stack size
                NULL,                   // Parameters
                PRECHARGE_PRIORITY,     // Priority
                NULL);                  // Task handle

    xTaskCreate(updateStatusTask, "UpdateStatusTask", UPDATE_STATUS_STACK_SIZE, NULL,
                UPDATE_STATUS_PRIORITY, NULL);
    
    xTaskCreate(monitorShutdownCircuitTask, "MonitorShutdownCircuitTask", SHUTDOWN_CIRCUIT_STACK_SIZE, NULL,
        SHUTDOWN_CIRCUIT_PRIORITY, NULL);
    
    vTaskStartScheduler();
}

void threadMain(void *pvParameters) {
}

void loop() {}
