// Anteater Electric Racing, 2025

#include <ADC.h>

// #include <ADC.h>
#include <iostream>
#include "peripherals/adc.h"

ADC *adc = new ADC();

void ADC0_Init() {
    adc->adc0->setAveraging(1); // set number of averages
    adc->adc0->setResolution(12); // set bits of resolution
    adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::LOW_SPEED); // change the conversion speed
    adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::LOW_SPEED); // change the sampling speed
}

void ADC1_Init() {
    adc->adc1->setAveraging(1); // set number of averages
    adc->adc1->setResolution(12); // set bits of resolution
    adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::LOW_SPEED); // change the conversion speed
    adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::LOW_SPEED); // change the sampling speed
    adc->adc1->enableInterrupts(callbacks);
}

void ADC1_start() {
    uint8_t currentPin = pins[adcIndex];
    std::cout << "Starting read for " << currentPin << std::endl;
    adc->adc1->startSingleRead(currentPin); // in callbacks.h
    adc->adc1->startTimer(1000); // every 0.001 seconds. starts single read every time period
}
