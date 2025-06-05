// Anteater Electric Racing, 2025

#define THREAD_CAN_STACK_SIZE 128
#define THREAD_CAN_PRIORITY 1

#include "can.h"
#include <FlexCAN_T4.h>
#include <arduino_freertos.h>

#define CAN_BAUD_RATE 500000
#define BCU_LEN 8 // bytes
#define TX 8
#define RX 7

FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_16> can3;
CAN_message_t rx_msg;

void CAN_Init() {
    // Initialize CAN bus
    can3.begin();
    can3.setBaudRate(CAN_BAUD_RATE);
    can3.setTX(TX);
    can3.setRX(RX);
    can3.enableFIFO();
    can3.enableFIFOInterrupt();
    can3.enableMBInterrupts();
    can3.setMaxMB(16);
}