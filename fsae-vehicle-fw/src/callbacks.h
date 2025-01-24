// Anteater Electric Racing, 2025

#include <cstdint>

static uint16_t pins[8] = {21, 24, 25, 19, 18, 14, 15, 17};
static uint16_t adcIndex = 0;
static uint16_t adcReads[8];

void callbacks ();
