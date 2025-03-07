// Anteater Electric Racing, 2025

#define THREAD_MAIN_STACK_SIZE 128
#define THREAD_MAIN_PRIORITY 1

#include <Arduino.h>
#include <FlexCAN_T4.h>
#include <arduino_freertos.h>

#include "peripherals/peripherals.h"
#include "peripherals/adc.h"

#include "vehicle/faults.h"
#include "vehicle/motor.h"
#include "vehicle/telemetry.h"

void threadMain( void *pvParameters );

FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_16> can3;
CAN_message_t msg;
CAN_message_t rx_msg;

void setup() { // runs once on bootup
    xTaskCreate(threadMain, "threadMain", THREAD_MAIN_STACK_SIZE, NULL, THREAD_MAIN_PRIORITY, NULL);
    vTaskStartScheduler();
}

void threadMain( void *pvParameters ) {
    Serial.begin(9600);

    Serial.println("In setup");
    Serial.println("Main task started");
    Serial.println("CANbus init");

    can3.begin();
    can3.setBaudRate(500000);
    can3.setTX(DEF);
    can3.setRX(DEF);
    // can3.setMaxMB(8);
    can3.enableFIFO();
    can3.enableFIFOInterrupt();
    can3.enableMBInterrupts();

    can3.setMaxMB(16);

    // msg.id = 0x101;
    // msg.len = 8;


    // can3.write(msg);
    Peripherals_Init();

    Faults_Init();
    Telemetry_Init();

    while (true) {
        // Serial.println("In main");
        // int res = can3.write(MB0, msg);
        // can3.mailboxStatus();
        // Serial.println(res);
        // msg.id++;
        // vTaskDelay(1000);
        // Main loop
    }
}

void loop() {
}
