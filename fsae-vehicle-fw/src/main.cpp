// Anteater Electric Racing, 2025

#define THREAD_MAIN_STACK_SIZE 128
#define THREAD_MAIN_PRIORITY 1

#include <Arduino.h>
#include <FlexCAN_T4.h>
#include <arduino_freertos.h>

#include "peripherals/adc.h"
#include "peripherals/peripherals.h"
#include "vehicle/faults.h"
#include "vehicle/telemetry.h"
#include "peripherals/watchdog.h"

volatile bool systemHealthy = false;

void threadMain( void *pvParameters );

void setup() { // runs once on bootup
    xTaskCreate(threadMain, "threadMain", THREAD_MAIN_STACK_SIZE, NULL, THREAD_MAIN_PRIORITY, NULL);
    vTaskStartScheduler();
}

void threadMain(void *pvParameters) {
    Serial.begin(9600);
    vTaskDelay(100);

    Peripherals_Init();
    Faults_Init();
    Telemetry_Init();
    Watchdog_Init();

    systemHealthy = true; // initialization was successful

    Serial.println("Entering loop...");

    while (true) {
        // Main loop
            Serial.println("Running...");

            /*
            if(some error condition){
                systemHealthy = false;
            }
            */

            if (systemHealthy){
                Watchdog_Pet(); //comment if testing
            }
            else{
                Serial.println("System not healthy.");
            }
            vTaskDelay(100);
    }
}


void loop() {}
