#pragma once
#include <cstdint>
void Shock_Init();

void ShockTravelUpdateData(uint16_t rawReading1, uint16_t rawReading2,
                            uint16_t rawReading3, uint16_t rawReading4);
