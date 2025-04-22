// Anteater Electric Racing, 2025

#include "peripherals/adc.h"
#include "callbacks.h"
#include "utils/utils.h"

void ADC1_Init() {
    adc->adc1->setAveraging(1); // set number of averages
    adc->adc1->setResolution(ADC_RESOLUTION); // set bits of resolution
    adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::LOW_SPEED); // change the conversion speed
    adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::LOW_SPEED); // change the sampling speed
}
