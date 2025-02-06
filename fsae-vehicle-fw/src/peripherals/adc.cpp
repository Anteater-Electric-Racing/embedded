// Anteater Electric Racing, 2025

#include <ADC.h>

#include "peripherals/adc.h"
#include "callbacks.h"

void ADC1_init() {
    adc->adc1->setAveraging(1); // set number of averages
    adc->adc1->setResolution(10); // set bits of resolution
    adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::LOW_SPEED); // change the conversion speed
    adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::LOW_SPEED); // change the sampling speed
    Serial.println("Done initializing ADCs");
}

uint32_t begin;

