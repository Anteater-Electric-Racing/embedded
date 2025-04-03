// Anteater Electric Racing, 2025

#define THREAD_CAN_STACK_SIZE 128
#define THREAD_CAN_PRIORITY 1

#include "can.h"
#include <FlexCAN_T4.h>
#include <arduino_freertos.h>


FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_16> can3;
CAN_message_t msg;
CAN_message_t rx_msg;

void threadCAN( void *pvParameters );

void CAN_Init() {
    // Initialize CAN bus
    can3.begin();
    can3.setBaudRate(500000);
    can3.setTX(DEF);
    can3.setRX(DEF);
    can3.enableFIFO();
    can3.enableFIFOInterrupt();
    can3.enableMBInterrupts();
    can3.setMaxMB(16);
}

void CAN_Begin() {
    xTaskCreate(threadCAN, "threadCAN", THREAD_CAN_STACK_SIZE, NULL, THREAD_CAN_PRIORITY, NULL);
    vTaskStartScheduler();
}

void threadCAN(void *pvParameters){
    while(true){
        int res = can3.write(MB0, msg);
        can3.mailboxStatus();
        Serial.println(res);
        msg.id++;
        vTaskDelay(1000);
    }
}


