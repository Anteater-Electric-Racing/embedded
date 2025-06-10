// Anteater Electric Racing, 2025

#pragma once

#include <stdint.h>

void CAN_Init();
void CAN_SendPCCMessage(uint8_t state, uint8_t errorCode, float accumulatorVoltage, float tsVoltage, float prechargeProgress);