// Anteater Electric Racing, 2025

#include <cstdint>
#include <chrono>

#define SENSOR_PIN_AMT 2
static uint16_t pins[SENSOR_PIN_AMT] = {18, 17}; // A4, A4, 18, 17, 17, 17, 17}; // real values: {21, 24, 25, 19, 18, 14, 15, 17};
static uint16_t adcIndex = 0;
static uint16_t adcReads[SENSOR_PIN_AMT];
static std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
static std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

void callbacks ();
