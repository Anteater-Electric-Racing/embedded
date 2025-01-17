// Anteater Electric Racing, 2025

#include <ADC.h>

#include "peripherals/adc.h"
#include "callbacks.h"

ADC *adc = new ADC();

void ADC1_init() {
    adc->adc1->setAveraging(1); // set number of averages
    adc->adc1->setResolution(12); // set bits of resolution
    adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::LOW_SPEED); // change the conversion speed
    adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::LOW_SPEED); // change the sampling speed
    adc->adc1->enableInterrupts(callbacks);
}

void ADC1_start() {
    adc->adc1->startSingleRead(pins[index]); // in callbacks.h
    adc->adc1->startTimer(1000); // every 0.001 seconds
}
