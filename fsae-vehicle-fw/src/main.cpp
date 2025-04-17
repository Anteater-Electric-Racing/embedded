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

void threadMain( void *pvParameters );

void setup() { // runs once on bootup
    xTaskCreate(threadMain, "threadMain", THREAD_MAIN_STACK_SIZE, NULL,
                THREAD_MAIN_PRIORITY, NULL);
    vTaskStartScheduler();
}

void threadMain(void *pvParameters) {
    Serial.begin(9600);

    Peripherals_Init();

    Faults_Init();
    Telemetry_Init();

    while (true) {
        // Main loop
    }
}

void loop() {}

// Update sensor last updates
/*
void updateBrakeSensor() {
    sensorLastUpdate.brake = millis();
}

void updateSuspensionSensor() {
    sensorLastUpdate.suspension = millis();
}

void updatePedalSensor() {
    sensorLastUpdate.pedal = millis();
}

void updateImuSensor() {
    sensorLastUpdate.imu = millis();
}

void updateGpsSensor() {
    sensorLastUpdate.gps = millis();
}
*/
