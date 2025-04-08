// Anteater Electric Racing, 2025

#include <ADC.h>
#define SENSOR_PIN_AMT_ADC0 8
#define SENSOR_PIN_AMT_ADC1 8

// Last updated ADC0 Pins on 4/8/25 based on pinouts in https://github.com/Anteater-Electric-Racing/Central-Computer-Module
static uint16_t adc0Pins[SENSOR_PIN_AMT_ADC0] = {19, 18, 17, 16, 23, 22, 21, 20};
static volatile uint16_t adc0Index = 0;
static volatile uint16_t adc0Reads[SENSOR_PIN_AMT_ADC0];

static uint16_t adc1Pins[SENSOR_PIN_AMT_ADC1] = {A7, A6, A5, A4, A3, A2, A1, A0}; // A4, A4, 18, 17, 17, 17, 17}; // real values: {21, 24, 25, 19, 18, 14, 15, 17};
static volatile uint16_t adc1Index = 0;
static volatile uint16_t adc1Reads[SENSOR_PIN_AMT_ADC1];

static ADC *adc = new ADC();

void ADC1_Init();
