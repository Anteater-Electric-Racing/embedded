// Anteater Electric Racing, 2025

#include "peripherals/adc.h"
#include "callbacks.h"

uint16_t adc0Pins[SENSOR_PIN_AMT_ADC0] = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7};
volatile uint16_t adc0Index = 0;
uint16_t adc0Reads[SENSOR_PIN_AMT_ADC0];

uint16_t adc1Pins[SENSOR_PIN_AMT_ADC1] = {0xA7, 0xA6, 0xA5, 0xA4, 0xA3, 0xA2, 0xA1, 0xA0};
volatile uint16_t adc1Index = 0;
uint16_t adc1Reads[SENSOR_PIN_AMT_ADC1];

ADC *adc = new ADC();

void ADC1_Init() {
    adc->adc1->setAveraging(1); // set number of averages
    adc->adc1->setResolution(10); // set bits of resolution
    adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::LOW_SPEED); // change the conversion speed
    adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::LOW_SPEED); // change the sampling speed
}
