#pragma once

#include <stdint.h>

typedef struct __attribute__((packed)) {
    int16_t dataPackCurrent;
    uint16_t dataPackVoltage;
    uint8_t batteryPct;
    uint8_t highestTempC;
    uint8_t avgTempC;
} BMSBatteryStatus;

typedef struct __attribute__((packed)) {
    uint8_t relay_status;
    uint8_t isolation_status;
    uint8_t flags[5];
} BMS_BPSStatus;

typedef struct __attribute__((packed)) {
    uint8_t error1;
    uint8_t error2;
    uint8_t error3;
} BMSErrors;

void BMS_Init();
static void BMSThread(void *pvParameters);
