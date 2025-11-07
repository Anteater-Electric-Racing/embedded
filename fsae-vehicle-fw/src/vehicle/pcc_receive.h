#pragma once

#include <stdint.h>

typedef struct __attribute__((packed)) {
    //uint32_t timestamp;
    uint8_t state;
    uint8_t errorCode;
    //float accumulatorVoltage;
    //float tsVoltage;
    float prechargeProgress;
} PCC;

static PCC pccData;

void processPCCMessage(uint64_t);
