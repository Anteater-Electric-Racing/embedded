// Anteater Electric Racing, 2025

#include <cstdint>

#define SENSOR_PIN_AMT 8
static uint16_t pins[SENSOR_PIN_AMT] = {18, 18, 18, 18, 17, 17, 17, 17}; // real values: {21, 24, 25, 19, 18, 14, 15, 17};
static uint16_t adcIndex = 0;
static uint16_t adcReads[SENSOR_PIN_AMT];

void callbacks ();
