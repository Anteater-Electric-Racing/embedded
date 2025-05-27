// Anteater Electric Racing, 2025

#pragma once

// #include <FlexCAN_T4.h>

// FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> can2;
// FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_16> can3;

void CAN_Init();
void CAN_Send(uint32_t id, uint64_t msg);
void CAN_Receive(uint32_t* rx_id, uint64_t* rx_data);
