// Anteater Electric Racing, 2025

#pragma once

#include <cstdint>

void CAN_Init();
void CAN_Send(std::uint32_t id, std::uint64_t msg);
void CAN_Receive(std::uint32_t *rx_id, std::uint64_t *rx_data);

void CAN_ISOTP_Send(std::uint32_t id, std::uint8_t *msg, std::uint16_t size);

bool CAN_IsBusHealthy(std::uint8_t bus);
void CAN_CheckHealth();
