// Anteater Electric Racing, 2025

#include <ADC.h>

#include "peripherals/adc.h"
#include "callbacks.h"
#include <chrono>

void ADC1_init() {


    adc->adc1->setAveraging(1); // set number of averages
    adc->adc1->setResolution(12); // set bits of resolution
    adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::ADACK_20); // change the conversion speed
    adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::LOW_SPEED); // change the sampling speed
    adc->adc1->enableInterrupts(callbacks);
    Serial.println("Done initializing ADCs");
}

void ADC1_start() {
    uint8_t currentPin = pins[adcIndex];
    // uint8_t startPin = pins[0]; // TODO: Add checks for this
    Serial.print( "Starting read for ");
    Serial.println(currentPin);

    adc->adc1->startSingleRead(currentPin); // in callbacks.h
    adc->adc1->startTimer(10000); // every 0.0001 seconds. each read takes this long
    begin = std::chrono::steady_clock::now();
}
