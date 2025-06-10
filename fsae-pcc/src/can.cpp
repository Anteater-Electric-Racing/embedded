// Anteater Electric Racing, 2025

#define THREAD_CAN_STACK_SIZE 128
#define THREAD_CAN_PRIORITY 1

#include "can.h"
#include <FlexCAN_T4.h>
#include <arduino_freertos.h>

#define CAN_BAUDRATE 500000
#define PCC_CAN_ID 0x123

typedef struct __attribute__((packed)) {
    uint8_t state;      // Precharge state
    uint8_t errorCode; // Error code
    uint16_t accumulatorVoltage; // Accumulator voltage in volts
    uint16_t tsVoltage; // Transmission side voltage in volts
    uint16_t prechargeProgress; // Precharge progress in percent
} PCC;

static PCC pccData;

FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> can2;
static CAN_message_t pccMsg;

void CAN_Init() {
    can2.begin();
    can2.setBaudRate(CAN_BAUDRATE); // Set CAN baud rate
    can2.setTX(DEF);
    can2.setRX(DEF);
    can2.enableFIFO();

    pccMsg.id = PCC_CAN_ID; // can change ID
}

void CAN_SendPCCMessage(uint8_t state, uint8_t errorCode, float accumulatorVoltage, float tsVoltage, float prechargeProgress) {
    pccData = {
        .state = state,
        .errorCode = errorCode,
        .accumulatorVoltage = uint16_t(accumulatorVoltage * 100),
        .tsVoltage = uint16_t(tsVoltage * 100),
        .prechargeProgress = uint16_t(prechargeProgress * 100),
    };

    memcpy(pccMsg.buf, &pccData, sizeof(PCC));
    can2.write(pccMsg);
}