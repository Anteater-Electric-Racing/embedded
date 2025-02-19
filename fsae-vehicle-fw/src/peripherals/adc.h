// Anteater Electric Racing, 2025

#include <cstdint>
#include <ADC.h>
#define SENSOR_PIN_AMT_ADC0 8
#define SENSOR_PIN_AMT_ADC1 8
enum SensorIndexesADC0 {
    APPS_1_INDEX,
    APPS_2_INDEX,
    BSE_1_INDEX,
    BSE_2_INDEX,
    SUSP_TRAV_LINPOT1,
    SUSP_TRAV_LINPOT2,
    SUSP_TRAV_LINPOT3,
    SUSP_TRAV_LINPOT4
};

enum SensorIndexesADC1 {
    APPS_1_INDEX2,
    APPS_2_INDEX2,
    BSE_1_INDEX2,
    BSE_2_INDEX2,
    SUSP_TRAV_LINPOT12,
    SUSP_TRAV_LINPOT22,
    SUSP_TRAV_LINPOT32,
    SUSP_TRAV_LINPOT42
};

// note: change inputs for potentiometers to A0 (14) and A1 (15)
static uint16_t adc0Pins[SENSOR_PIN_AMT_ADC0] = {A0, A1, A2, A3, A4, A5, A6, A7}; // A4, A4, 18, 17, 17, 17, 17}; // real values: {21, 24, 25, 19, 18, 14, 15, 17};
static volatile uint16_t adc0Index = 0;
static volatile uint16_t adc0Reads[SENSOR_PIN_AMT_ADC0];

static uint16_t adc1Pins[SENSOR_PIN_AMT_ADC1] = {A7, A6, A5, A4, A3, A2, A1, A0}; // A4, A4, 18, 17, 17, 17, 17}; // real values: {21, 24, 25, 19, 18, 14, 15, 17};
static volatile uint16_t adc1Index = 0;
static volatile uint16_t adc1Reads[SENSOR_PIN_AMT_ADC1];



static ADC *adc = new ADC();

void ADC1_init();
