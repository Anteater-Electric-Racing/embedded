#define THREAD_CAN_STACK_SIZE 128
#define THREAD_CAN_PRIORITY 1

#include "pcc_receive.h"
#include "arduino_freertos.h"

static PCC pccData;

void PCC_Init() {
    pccData = {.state = 0,
               .errorCode = 0,
               .accumulatorVoltage = 0,
               .tsVoltage = 0,
               .prechargeProgress = 404};
}

void processPCCMessage(uint64_t rx_data) {
    taskENTER_CRITICAL();
    memcpy(&pccData, &rx_data, sizeof(PCC));
    taskEXIT_CRITICAL();

    // Serial.print("Precharge Progress: ");
    // Serial.println(pccData.prechargeProgress);
}

PCC *PCC_GetData() { return &pccData; }
