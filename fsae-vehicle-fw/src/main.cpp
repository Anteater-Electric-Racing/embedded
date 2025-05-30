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
    MCU_Init();

    Motor_Init();

    float torqueDemand = 0;
    TickType_t xLastWakeTime = xTaskGetTickCount(); // Initialize the last wake time

    bool enablePrecharge = false;
    bool enablePower = false;
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
                    enablePower = false; // Disable run state
                    enableRun = false; // Disable run state
                    torqueDemand = 0; // Reset torque demand
                    // Serial.println("Entering precharge state...");
                }
                    break;
                case 'o': // Power Ready state
                case 'O':
                    {
                        enablePrecharge = false; // Disable precharging
                        enablePower = true; // Set flag to enable power ready state
                        enableRun = false; // Set flag to enable run state
                    }
                break;

                case 'i':
                case 'I': // Run state
                    {
                        enablePrecharge = false; // Disable precharging
                        enablePower = false; // Set flag to enable power ready state
                        enableRun = true; // Set flag to enable run state
                    }
                    break;

                case ' ': // Stop all torque
                    torqueDemand = 0;
                    enableRun = false; // Disable run state
                    enablePrecharge = false; // Disable precharging
                    break;
                case 'f': // Fault state
                case 'F':
                    enableRun = false; // Disable run state
                    enablePrecharge = false; // Disable precharging
                    torqueDemand = 0; // Reset torque demand
                    Motor_SetFaultState(); // Set motor to fault state
                    break;
                default:
                    break;
            }
        }


        // Serial.print("State: ");
        // Serial.print(MCU_GetMCU1Data().mcuMainState);
        // Serial.print(" | ");
        // Serial.print("Internal State: ");
        // Serial.print(Motor_GetState());
        //Serial.print("      \n");


        // Serial.print("Torque - ");
        // Serial.print(torqueDemand);
        // Serial.print("      \n");

        // Telemetry: Read battery current, phase current, motor speed, temperature(s)
        Serial.print("State: ");
        Serial.print(MCU_GetMCU1Data().mcuMainState);
        Serial.print(" | ");
        Serial.print("Internal State: ");
        Serial.print(Motor_GetState());
        Serial.print("  | ");

        Serial.print("Torque: ");
        Serial.print(torqueDemand);
        Serial.print("  |  ");
        Serial.print("Motor Speed: ");
        Serial.print(MCU_GetMCU1Data().motorSpeed);

        // Telemetry: Read battery current, phase current, motor speed, temperature(s)
        Serial.print("  |  ");
        Serial.print("Battery Voltage: ");
        Serial.print(MCU_GetMCU3Data().mcuVoltage);
        Serial.print("  |  ");
        Serial.print("Battery Current: ");
        Serial.print(MCU_GetMCU3Data().mcuCurrent);
        Serial.print("  | ");
        Serial.print("MCU Warning Levl: ");
        Serial.print(MCU_GetMCU2Data().mcuWarningLevel);
        
        // Serial.print("Battery Current: ");
        // Serial.print(MCU_GetMCU3Data().mcuCurrent);
        // Serial.print("      \n");
        
        // Serial.print("Phase Current: ");
        // Serial.print(MCU_GetMCU3Data().motorPhaseCurr);
        // Serial.print("      \n");
        
        // Serial.print("MCU Temp: ");
        // Serial.print(MCU_GetMCU2Data().mcuTemp);
        // Serial.print("      \n");
        
        // Serial.print("Motor Temp: ");
        // Serial.print(MCU_GetMCU2Data().motorTemp);
        // Serial.print("      \n");
        
        // print all errors if they are true in one line
        // Serial.print("  |  ");
        // if (MCU_GetMCU2Data().dcMainWireOverVoltFault) Serial.print("DC Over Volt Fault, ");
        // if (MCU_GetMCU2Data().motorPhaseCurrFault) Serial.print("Motor Phase Curr Fault, ");
        // if (MCU_GetMCU2Data().mcuOverHotFault) Serial.print("MCU Over Hot Fault, ");
        // if (MCU_GetMCU2Data().resolverFault) Serial.print("Resolver Fault, ");
        // if (MCU_GetMCU2Data().phaseCurrSensorFault) Serial.print("Phase Curr Sensor Fault, ");
        // if (MCU_GetMCU2Data().motorOverSpdFault) Serial.print("Motor Over Spd Fault, ");
        // if (MCU_GetMCU2Data().drvMotorOverHotFault) Serial.print("Driver Motor Over Hot Fault, ");
        // if (MCU_GetMCU2Data().dcMainWireOverCurrFault) Serial.print("DC Main Wire Over Curr Fault, ");
        // if (MCU_GetMCU2Data().drvMotorOverCoolFault) Serial.print("Driver Motor Over Cool Fault, ");
        // if (MCU_GetMCU2Data().dcLowVoltWarning) Serial.print("DC Low Volt Warning, ");
        // if (MCU_GetMCU2Data().mcu12VLowVoltWarning) Serial.print("MCU 12V Low Volt Warning, ");
        // if (MCU_GetMCU2Data().motorStallFault) Serial.print("Motor Stall Fault, ");
        // if (MCU_GetMCU2Data().motorOpenPhaseFault) Serial.print("Motor Open Phase Fault, ");

        Serial.print("             \r");
        Motor_UpdateMotor(torqueDemand, enablePrecharge, enablePower, enableRun); // Update motor with the current torque demand

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10)); // Delay for 100ms
    }
}

void loop() {}

