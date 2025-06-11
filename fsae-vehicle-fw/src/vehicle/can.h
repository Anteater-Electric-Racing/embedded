// Anteater Electric Racing, 2025

#pragma once

void threadCAN(void *pvParameters);

void CAN_Init();
void CAN_Send(uint32_t id, uint64_t msg);
void CAN_Receive(uint32_t* rx_id, uint64_t* rx_data);

void CAN_ISOTP_Send(uint32_t id, uint8_t* msg, uint16_t size);
