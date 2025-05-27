// Anteater Electric Racing, 2025


#include <Arduino.h>
#include <arduino_freertos.h>

#include "peripherals/adc.h"
#include "peripherals/can.h"

#include "vehicle/apps.h"
#include "vehicle/bse.h"
#include "vehicle/faults.h"
#include "vehicle/motor.h"
#include "vehicle/telemetry.h"
#include "utils/utils.h"

void threadMain(void *pvParameters);

void setup() { // runs once on bootup
    ADC_Init();
    CAN_Init();
    APPS_Init();
    BSE_Init();
    Faults_Init();
    Telemetry_Init();
    Motor_Init();

    xTaskCreate(threadADC, "threadADC", THREAD_ADC_STACK_SIZE, NULL, THREAD_ADC_PRIORITY, NULL);
    xTaskCreate(threadMotor, "threadMotor", THREAD_MOTOR_STACK_SIZE, NULL, THREAD_MOTOR_PRIORITY, NULL);
    xTaskCreate(threadTelemetryCAN, "threadTelemetryCAN", THREAD_CAN_TELEMETRY_STACK_SIZE, NULL, THREAD_CAN_TELEMETRY_PRIORITY, NULL);
    xTaskCreate(threadMain, "threadMain", THREAD_MAIN_STACK_SIZE, NULL, THREAD_MAIN_PRIORITY, NULL);
    vTaskStartScheduler();
}

void threadMain(void *pvParameters) {
    Serial.begin(9600);



    while (true) {
        # if DEBUG_FLAG
            TelemetryData const* telem = Telemetry_GetData();
            Serial.print(telem->APPS_Travel, 4);
            Serial.print(" ");
            Serial.print(telem->debug[1], 4);
            Serial.print(" ");
            Serial.print("Motor state is ");
            Serial.println(telem->motorState);
            Serial.println();
        # endif
        vTaskDelay(50);
    }
}

void loop() {}
