#pragma once

#include <stdint.h>

typedef struct __attribute__((packed)) {
    //uint32_t timestamp;
    uint8_t state;
    uint8_t errorCode;
    uint16_t accumulatorVoltage;
    uint16_t tsVoltage;
    uint16_t prechargeProgress;
} PCC;

//static PCC pccData;

void PCC_Init();
void processPCCMessage(uint64_t);

PCC* PCC_GetData();
