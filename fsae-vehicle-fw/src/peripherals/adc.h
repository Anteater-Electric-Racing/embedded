// Anteater Electric Racing, 2025

#include <cstdint>
#include <ADC.h>
#define SENSOR_PIN_AMT 8
enum SensorIndexes {
    APPS_1_INDEX,
    APPS_2_INDEX,
    BSE_1_INDEX,
    BSE_2_INDEX,
    SUSP_TRAV_LINPOT1,
    SUSP_TRAV_LINPOT2,
    SUSP_TRAV_LINPOT3,
    SUSP_TRAV_LINPOT4
};

// note: change inputs for potentiometers to A0 (14) and A1 (15)
static uint16_t pins[SENSOR_PIN_AMT] = {A0, A1, A2, A3, A4, A5, A6, A7}; // A4, A4, 18, 17, 17, 17, 17}; // real values: {21, 24, 25, 19, 18, 14, 15, 17};
static uint16_t adcIndex = 0;
static uint16_t adcReads[SENSOR_PIN_AMT];

static ADC *adc = new ADC();

void ADC1_init();
