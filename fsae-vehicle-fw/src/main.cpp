// Anteater Electric Racing, 2025

#include <Arduino.h>
#include <arduino_freertos.h>

#include "peripherals/adc.h"
#include "peripherals/can.h"

#include "vehicle/apps.h"
#include "vehicle/bse.h"
#include "vehicle/faults.h"
#include "vehicle/ifl100-36.h"
#include "vehicle/motor.h"
#include "vehicle/telemetry.h"
#include "peripherals/gpio.h"
#include "vehicle/rtm_button.h"

#include "utils/utils.h"
#include <iostream>
#include <unistd.h>

#define TORQUE_STEP 1
#define TORQUE_MAX_NM 20 // Maximum torque demand in Nm

static TickType_t xLastWakeTime;

void threadMain(void *pvParameters);

void setup() { // runs once on bootup
    ADC_Init();
    CAN_Init();
    APPS_Init();
    BSE_Init();
    Faults_Init();
    Telemetry_Init();
    Motor_Init();
    MCU_Init();
    GPIO_Init();

    xTaskCreate(threadADC, "threadADC", THREAD_ADC_STACK_SIZE, NULL,
                THREAD_ADC_PRIORITY, NULL);
    xTaskCreate(threadMotor, "threadMotor", THREAD_MOTOR_STACK_SIZE, NULL,
                THREAD_MOTOR_PRIORITY, NULL);
    xTaskCreate(threadTelemetry, "threadTelemetryCAN",
                THREAD_CAN_TELEMETRY_STACK_SIZE, NULL,
                THREAD_CAN_TELEMETRY_PRIORITY, NULL);
    xTaskCreate(threadMain, "threadMain", THREAD_MAIN_STACK_SIZE, NULL,
                THREAD_MAIN_PRIORITY, NULL);
    vTaskStartScheduler();
}

void threadMain(void *pvParameters) {
    Serial.begin(9600);
    xLastWakeTime = xTaskGetTickCount(); // Initialize the last wake time

#if HIMAC_FLAG
    float torqueDemand = 0;
    bool enableStandby = false;
    bool enablePrecharge = false;
    bool enablePower = false;
    bool enableRun = false;

    bool enableRegen = false;

#endif
    while (true) {
        digitalWrite(13, 1);
        /*
         * Read user input from Serial to control torque demand.
         * 'w' or 'W' to increase torque demand,
         * 's' or 'S' to decrease torque demand,
         *
         * 'p' or 'P' to enter precharging state from standby,
         * 'l' or 'L' to enter standby from precharging
         *
         * 'o' or 'O' to enter run state,
         * 'k' or 'K' to go back to idle from run
         *
         * ' ' (space) to stop all torque. (reset w/s to 0)
         * R to toggle regen
         *
         * The torque demand is limited between 0 and TORQUE_MAX_NM.
         *
         * Telemetry: battery current, phase current, motor speed,
         * temperature(s)
         */

#if HIMAC_FLAG
        if (Serial.available()) {
            char input = Serial.read();

            switch (input) {
            case 'w': // Increase torque demand
            case 'W': {
                if (torqueDemand < TORQUE_MAX_NM) {
                    torqueDemand += TORQUE_STEP; // Increment torque demand
                }
                break;
            }
            case 's': // Decrease torque demand
            case 'S': {
                if (torqueDemand > 0) {
                    torqueDemand -= TORQUE_STEP; // Decrement torque demand
                }
                break;
            }
            case 'l':
            case 'L': { // Standby state
                enableStandby = true;
                enablePrecharge = false; // Disable Precharge
                enablePower = false;     // Disable run state
                enableRun = false;       // Disable run state
                break;
            }
            case 'p': // Precharge state
            case 'P': {
                enableStandby = false;
                enablePrecharge = true; // Set flag to enable precharging
                enablePower = false;    // Disable run state
                enableRun = false;      // Disable run state
                torqueDemand = 0;       // Reset torque demand
                // Serial.println("Entering precharge state...");
                break;
            }
            case 'o': // IDLE: Power Ready state
            case 'O': {
                enableStandby = false;
                enablePrecharge = false; // Disable precharging
                enablePower = true;      // Set flag to enable power ready state
                enableRun = false;       // Set flag to enable run state
                break;
            }
            case 'i':
            case 'I': { // Run state
                enableStandby = false;
                enablePrecharge = false; // Disable precharging
                enablePower = false;     // Set flag to enable power ready state
                enableRun = true;        // Set flag to enable run state
                break;
            }
            case ' ': { // Stop all torque
                torqueDemand = 0;
                // enableRun = false;       // Disable run state
                // enablePrecharge = false; // Disable precharging
                // enablePower = false;
                // enableRun = false;
                break;
            }
            case 'f': // Fault state
            case 'F': {
                //Serial.println("Entering fault state");
                enableRun = false;       // Disable run state
                enablePrecharge = false; // Disable precharging
                enablePower = false;
                torqueDemand = 0;        // Reset torque demand
                Motor_SetFaultState();   // Set motor to fault state
                break;
            }
            case 'r':
            case 'R': {
                enableRegen = !enableRegen;
                break;
            }
            default:
                break;
            }
        }

        // Serial.print("State: ");
        // Serial.print(MCU_GetMCU1Data().mcuMainState);
        // Serial.print(" | ");
        // Serial.print("Internal State: ");
        // Serial.print(Motor_GetState());
        // Serial.print("      \n");

        // Serial.print("Torque - ");
        // Serial.print(torqueDemand);
        // Serial.print("      \n");

        // Telemetry: Read battery current, phase current, motor speed,
        // temperature(s)
        Serial.print("C State: ");
        Serial.print(MCU_GetMCU1Data()->mcuMainState);
        Serial.print(" | ");
        // Serial.print("APPS: ");
        // Serial.print(APPS_GetAPPSReading1());
        // Serial.print(" | ");
        // Serial.print(APPS_GetAPPSReading2());
        // Serial.print(" | ");
        Serial.print("T State: ");
        Serial.print(Motor_GetState());
        Serial.print(" | ");

        Serial.print("Torque: ");
        Serial.print(torqueDemand);
        // Serial.print(" | ");
        // Serial.print("M: ");
        // Serial.print(MCU_GetMCU1Data()->motorTorque);
        Serial.print(" | ");
        Serial.print("RPM: ");
        Serial.print(MCU_GetMCU1Data()->motorSpeed);


        //Serial.print(" | ");
        // Serial.print("maxtorq ");
        // Serial.print(MCU_GetMCU1Data()->maxMotorTorque);

        // Telemetry: Read battery current, phase current, motor speed,
        // temperature(s)
        Serial.print(" | ");
        Serial.print("B Volt: ");
        Serial.print(MCU_GetMCU3Data()->mcuVoltage);
        Serial.print(" | ");
        Serial.print("B Curr: ");
        Serial.print(MCU_GetMCU3Data()->mcuCurrent);
        Serial.print(" | ");
        Serial.print("P Curr: ");
        Serial.print(MCU_GetMCU3Data()->motorPhaseCurr);
        Serial.print(" | ");
        Serial.print("WarnLvl: ");
        Serial.print(MCU_GetMCU2Data()->mcuWarningLevel);
        Serial.print(" | ");
        Serial.print("MCU Temp: ");
        Serial.print(MCU_GetMCU2Data()->mcuTemp);
        Serial.print(" | ");
        Serial.print("Mtr Temp: ");
        Serial.print(MCU_GetMCU2Data()->motorTemp);


        // Serial.print(" | ");
        // Serial.print("Regen: ");
        // Serial.print(enableRegen);
        Serial.print("\r");




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
        Serial.print("  |  ");

        if (MCU_GetMCU2Data()->dcMainWireOverVoltFault)
            Serial.println("DC Over Volt Fault, ");
        if (MCU_GetMCU2Data()->motorPhaseCurrFault)
            Serial.println("Motor Phase Curr Fault, ");
        if (MCU_GetMCU2Data()->mcuOverHotFault)
            Serial.println("MCU Over Hot Fault, ");
        if (MCU_GetMCU2Data()->resolverFault)
            Serial.println("Resolver Fault, ");
        if (MCU_GetMCU2Data()->phaseCurrSensorFault)
            Serial.println("Phase Curr Sensor Fault, ");
        if (MCU_GetMCU2Data()->motorOverSpdFault)
            Serial.println("Motor Over Spd Fault, ");
        if (MCU_GetMCU2Data()->drvMotorOverHotFault)
            Serial.println("Driver Motor Over Hot Fault, ");
        if (MCU_GetMCU2Data()->dcMainWireOverCurrFault)
            Serial.println("DC Main Wire Over Curr Fault, ");
        if (MCU_GetMCU2Data()->drvMotorOverCoolFault)
            Serial.println("Driver Motor Over Cool Fault, ");
        if (MCU_GetMCU2Data()->dcLowVoltWarning)
            Serial.println("DC Low Volt Warning, ");
        if (MCU_GetMCU2Data()->mcu12VLowVoltWarning)
            Serial.println("MCU 12V Low Volt Warning, ");
        if (MCU_GetMCU2Data()->motorStallFault)
            Serial.println("Motor Stall Fault, ");
        if (MCU_GetMCU2Data()->motorOpenPhaseFault)
            Serial.println("Motor Open Phase Fault, ");




        Motor_UpdateMotor(
            torqueDemand, enablePrecharge, enablePower, enableRun,
            enableRegen, enableStandby); // Update motor with the current torque demand


#endif
        Serial.print(RTMButton_GetState());
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10)); // Delay for 100ms
    }
}

void loop() {}
