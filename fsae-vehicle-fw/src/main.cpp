// Anteater Electric Racing, 2025

// #define THREAD_MAIN_STACK_SIZE 128
// #define THREAD_MAIN_PRIORITY 1

// #include <Arduino.h>
#include <FlexCAN_T4.h>
// #include <arduino_freertos.h>

// #include "peripherals/peripherals.h"

// #include "vehicle/faults.h"
// #include "vehicle/motor.h"
// #include "vehicle/telemetry.h"

// void threadMain( void *pvParameters );

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;
CAN_message_t msg;
CAN_message_t rx_msg;

void setup() {
    // xTaskCreate(threadMain, "threadMain", THREAD_MAIN_STACK_SIZE, NULL, THREAD_MAIN_PRIORITY, NULL);

    Serial.begin(9600);

    delay(1000);

    can1.begin();
    can1.setBaudRate(125000);
    can1.setTX(DEF);
    can1.setRX(DEF);
    // can1.enableLoopBack();

    // can1.setMaxMB(16);
    // can1.enableFIFO();
    // can1.enableFIFOInterrupt();

    msg.id = 0x101;
    msg.len = 8;

    Serial.println("CANbus init");
    // can1.write(msg);
}

// void threadMain( void *pvParameters ) {
//     Peripherals_Init();

//     Faults_Init();
//     Telemetry_Init();

//     while (true) {
//         // Main loop
//     }
// }

void loop() {
    can1.read(rx_msg);
    int res = can1.write(msg);
    can1.mailboxStatus();
    Serial.println(res);
    Serial.println(rx_msg.id);
    delay(1000);
}
