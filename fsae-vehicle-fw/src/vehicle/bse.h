// Anteater Electric Racing, 2025
#pragma once

#include <stdint.h>

typedef struct {
    float bseFront_PSI; // front brake pressure in PSI
    float bseRear_PSI;  // rear brake pressure in PSI
} BSEData;

void BSE_Init();
void BSE_UpdateData(uint32_t bseReading1, uint32_t bseReading2);
BSEData *BSE_GetBSEReading();
