// Anteater Electric Racing, 2025

#include "peripherals/adc.h"
#include "callbacks.h"

uint16_t adc0Pins[SENSOR_PIN_AMT_ADC0] = {A0, A1, A2, A3, A4, A5, A6, A7};
volatile uint16_t adc0Index = 0;
uint16_t adc0Reads[SENSOR_PIN_AMT_ADC0];

uint16_t adc1Pins[SENSOR_PIN_AMT_ADC1] = {A7, A6, A5, A4, A3, A2, A1, A0};
volatile uint16_t adc1Index = 0;
uint16_t adc1Reads[SENSOR_PIN_AMT_ADC1];

ADC *adc = new ADC();

void ADC1_Init() {
    adc->adc1->setAveraging(1); // set number of averages
    adc->adc1->setResolution(10); // set bits of resolution
    adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::LOW_SPEED); // change the conversion speed
    adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::LOW_SPEED); // change the sampling speed
}
