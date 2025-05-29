// Anteater Electric Racing, 2025

#define THREAD_MAIN_STACK_SIZE 128
#define THREAD_MAIN_PRIORITY 5

#include <Arduino.h>
#include <arduino_freertos.h>

#include "peripherals/adc.h"
#include "peripherals/peripherals.h"
#include "peripherals/can.h"

#include "vehicle/apps.h"
#include "vehicle/bse.h"
#include "vehicle/faults.h"
#include "vehicle/motor.h"
#include "vehicle/telemetry.h"

#include <iostream>
#include <unistd.h>
#define TORQUE_STEP 0.1F

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

    Motor_Init();

    float torqueDemand = 0.0F;

    while (true) {
        if (Serial.available()) {
            char input = Serial.read();

            switch (input) {
                case 'w': // Increase torque demand
                case 'W':
                    torqueDemand += TORQUE_STEP; // Increment torque demand
                    Motor_UpdateMotor();
                    Serial.print("Torque - ");
                    Serial.print(torqueDemand);
                    Serial.print("\r");
                    break;
                case 's': // Decrease torque demand
                case 'S':
                    torqueDemand -= TORQUE_STEP; // Decrement torque demand
                    Motor_UpdateMotor();
                    Serial.print("Torque - ");
                    Serial.print(torqueDemand);
                    Serial.print("\r");
                    break;
                case ' ': // Stop all torque
                    Motor_UpdateMotor();
                    torqueDemand = 0.0F;
                    Serial.print("Torque - 0.0");
                    Serial.print("\r");
                    break;
                default:
                    Serial.print("Unknown command. (W, S, space)");
                    Serial.print("\r");
                    break;
            }
        }
    }
}

void loop() {}

