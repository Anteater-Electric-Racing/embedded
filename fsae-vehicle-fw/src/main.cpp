// Anteater Electric Racing, 2025

#define THREAD_MAIN_STACK_SIZE 128
#define THREAD_MAIN_PRIORITY 1

#include <Arduino.h>
#include <FlexCAN_T4.h>
#include <arduino_freertos.h>

#include "peripherals/adc.h"
#include "peripherals/peripherals.h"

#include "vehicle/bse.h"
#include "vehicle/faults.h"
#include "vehicle/motor.h"
#include "vehicle/telemetry.h"

void threadMain(void *pvParameters);

void setup() { // runs once on bootup
    xTaskCreate(threadMain, "threadMain", THREAD_MAIN_STACK_SIZE, NULL,
                THREAD_MAIN_PRIORITY, NULL);
    vTaskStartScheduler();
}

void threadMain(void *pvParameters) {
    Serial.begin(9600);

    Peripherals_Init();

    BSE_Init();

    Faults_Init();
    Telemetry_Init();

    while (true) {
        TelemetryData const* telem = Telemetry_GetData();
        Serial.print(telem->APPS_Travel);
        Serial.print(" ");
        Serial.print(telem->motorState);
        Serial.println();
        vTaskDelay(100);
    }
}

void loop() {}
