// Anteater Electric Racing, 2025

// #define THREAD_MAIN_STACK_SIZE 128
// #define THREAD_MAIN_PRIORITY 1

#include <Arduino.h>
#include <FlexCAN_T4.h>
// #include <arduino_freertos.h>

// #include "peripherals/peripherals.h"

// #include "vehicle/faults.h"
// #include "vehicle/motor.h"
// #include "vehicle/telemetry.h"

// void threadMain( void *pvParameters );

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;
CAN_message_t msg;

void setup(void) {
    // xTaskCreate(threadMain, "threadMain", THREAD_MAIN_STACK_SIZE, NULL, THREAD_MAIN_PRIORITY, NULL);

    Serial.begin(9600);
    while (!Serial);

    can1.begin();
    can1.setBaudRate(500000);

    msg.id = 0x101;
    msg.len = 8;

    Serial.println("CANbus init");
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
    can1.write(msg);
    Serial.println("Message sent");
    delay(1000);
}
