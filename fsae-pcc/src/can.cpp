// Anteater Electric Racing, 2025

#define THREAD_CAN_STACK_SIZE 128
#define THREAD_CAN_PRIORITY 1

#include "can.h"
#include <FlexCAN_T4.h>
#include <arduino_freertos.h>

#define CAN_BAUDRATE 500000
#define PCC_CAN_ID 0x123

typedef struct __attribute__((packed)) {
    // uint32_t timestamp; // Timestamp in milliseconds
    uint8_t state;     // Precharge state
    uint8_t errorCode; // Error code
    // float accumulatorVoltage; // Accumulator voltage in volts
    // float tsVoltage; // Transmission side voltage in volts
    float prechargeProgress; // Precharge progress in percent
} PCC;

static PCC pccData;

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;
static CAN_message_t pccMsg;

void CAN_Init() {
    can1.begin();
    can1.setBaudRate(CAN_BAUDRATE); // Set CAN baud rate
    can1.setTX(DEF);
    can1.setRX(DEF);
    can1.enableFIFO();

    pccMsg.id = PCC_CAN_ID; // can change ID
}

void CAN_SendPCCMessage(uint32_t timestamp, uint8_t state, uint8_t errorCode,
                        float accumulatorVoltage, float tsVoltage,
                        float prechargeProgress) {
    pccData = {
        //.timestamp = timestamp, 4 bytes
        .state = state,         // 1 bytes
        .errorCode = errorCode, // 1 bytes
        //.accumulatorVoltage = accumulatorVoltage, 4 bytes
        //.tsVoltage = tsVoltage, 4 bytes
        .prechargeProgress = prechargeProgress // 4 bytes
    };

    // Serial.print("PCC Message: ");
    // Serial.print("Timestamp: ");
    // Serial.print(pccData.timestamp);
    // Serial.print(", State: ");
    // Serial.print(pccData.state);
    // Serial.print(", Error Code: ");
    // Serial.print(pccData.errorCode);
    // Serial.print(" | ACCV: ");
    // Serial.print(pccData.accumulatorVoltage);
    // Serial.print(" | TSV: ");
    // Serial.print(pccData.tsVoltage);
    // Serial.print(" | Progress: ");
    // Serial.println(pccData.prechargeProgress);

    memcpy(pccMsg.buf, &pccData, sizeof(PCC));
    can1.write(pccMsg);
}
