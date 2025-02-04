// Anteater Electric Racing, 2025

#include <cstdint>
#include <chrono>

#define SENSOR_PIN_AMT 8
#define APPS_1_INDEX 0
#define APPS_2_INDEX 1
#define BSE_1_INDEX 2
#define BSE_2_INDEX 3
#define SUSP_TRAV_LINPOT1 4
#define SUSP_TRAV_LINPOT2 5
#define SUSP_TRAV_LINPOT3 6
#define SUSP_TRAV_LINPOT4 7

// note: change inputs for potentiometers to A0 (14) and A1 (15)
static uint16_t pins[SENSOR_PIN_AMT] = {A0, A1, A2, A3, A4, A5, A6, A7}; // A4, A4, 18, 17, 17, 17, 17}; // real values: {21, 24, 25, 19, 18, 14, 15, 17};
static uint16_t adcIndex = 0;
static uint16_t adcReads[SENSOR_PIN_AMT];

extern uint32_t begin; // to track duration of cycles, currently just used for info

void callbacks ();
