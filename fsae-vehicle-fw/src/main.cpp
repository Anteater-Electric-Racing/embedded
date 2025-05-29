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
#include "vehicle/ifl100-36.h"

#include <iostream>
#include <unistd.h>

#define TORQUE_STEP 1
#define TORQUE_MAX_NM 20 // Maximum torque demand in Nm

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

    float torqueDemand = 0;
    TickType_t xLastWakeTime = xTaskGetTickCount(); // Initialize the last wake time

    bool enablePrecharge = false;
    bool enableRun = false;

    while (true) {
        /*
            * Read user input from Serial to control torque demand.
            * 'w' or 'W' to increase torque demand,
            * 's' or 'S' to decrease torque demand,
            * 'p' or 'P' to enter precharging state from standby,
            * 'o' or 'O' to enter run state,
            * ' ' (space) to stop all torque.
            * The torque demand is limited between 0 and TORQUE_MAX_NM.
            *
            * Telemetry: battery current, phase current, motor speed, temperature(s)
        */
        if (Serial.available()) {
            char input = Serial.read();

            switch (input) {
                case 'w': // Increase torque demand
                case 'W':
                {
                    if (torqueDemand < TORQUE_MAX_NM) {
                        torqueDemand += TORQUE_STEP; // Increment torque demand
                    }
                    break;
                }
                case 's': // Decrease torque demand
                case 'S':
                {
                    if( torqueDemand > 0) {
                        torqueDemand -= TORQUE_STEP; // Decrement torque demand
                    }
                    break;
                }
                case 'p': // Precharge state
                case 'P':
                {
                    enablePrecharge = true; // Set flag to enable precharging
                    enableRun = false; // Disable run state
                    torqueDemand = 0; // Reset torque demand
                }
                    break;
                case 'o': // Run state
                case 'O':
                    {
                        enableRun = true; // Set flag to enable run state
                        enablePrecharge = false; // Disable precharging
                    }
                break;

                case ' ': // Stop all torque
                    torqueDemand = 0;
                    enableRun = false; // Disable run state
                    enablePrecharge = false; // Disable precharging
                    break;
                default:
                    break;
            }
        }

        Serial.print("Torque - ");
        Serial.print(torqueDemand);
        Serial.print("      \r");

        //Telemetry: Read battery current, phase current, motor speed, temperature(s)
        Serial.print("Battery Voltage: ");
        Serial.print(MCU_GetMCU3Data().mcuVoltage);
        Serial.print("      \r");

        Serial.print("Battery Current: ");
        Serial.print(MCU_GetMCU3Data().mcuCurrent);
        Serial.print("      \r");

        Serial.print("Phase Current: ");
        Serial.print(MCU_GetMCU3Data().motorPhaseCurr);
        Serial.print("      \r");

        Serial.print("Motor Speed: ");
        Serial.print(MCU_GetMCU1Data().motorSpeed);
        Serial.print("      \r");

        Serial.print("MCU Temp: ");
        Serial.print(MCU_GetMCU2Data().mcuTemp);
        Serial.print("      \r");

        Serial.print("Motor Temp: ");
        Serial.print(MCU_GetMCU2Data().motorTemp);
        Serial.print("      \r");

        Motor_UpdateMotor(torqueDemand, enablePrecharge, enableRun); // Update motor with the current torque demand

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1)); // Delay for 100ms
    }
}

void loop() {}

