// Anteater Electric Racing, 2025

#define THREAD_MAIN_STACK_SIZE 128
#define THREAD_MAIN_PRIORITY 5

#include <Arduino.h>
#include <arduino_freertos.h>

#include "peripherals/adc.h"
#include "peripherals/peripherals.h"

#include "vehicle/apps.h"
#include "vehicle/bse.h"
#include "vehicle/faults.h"
#include "vehicle/motor.h"
#include "vehicle/telemetry.h"

void threadMain(void *pvParameters);

void setup() { // runs once on bootup
    xTaskCreate(threadMain, "threadMain", THREAD_MAIN_STACK_SIZE, NULL, THREAD_MAIN_PRIORITY, NULL);
    vTaskStartScheduler();
}

void threadMain(void *pvParameters) {
    Serial.begin(9600);

    Peripherals_Init();

    APPS_Init();
    BSE_Init();

    Faults_Init();
    Telemetry_Init();

    while (true) {
        TelemetryData const* telem = Telemetry_GetData();
        // Serial.print(telem->APPS_Travel, 4);
        // Serial.print(" ");
        // Serial.print(telem->debug[1], 4);
        // Serial.print(" ");
        Serial.print(telem->motorState);
        Serial.println();
        vTaskDelay(50);
    }
}

void loop() {}
