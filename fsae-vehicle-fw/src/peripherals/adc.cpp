// Anteater Electric Racing, 2025

#include <ADC.h>

#include "peripherals/adc.h"
#include "callbacks.h"

void ADC1_init() {
    adc->adc1->setAveraging(1); // set number of averages
    adc->adc1->setResolution(12); // set bits of resolution
    adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::LOW_SPEED); // change the conversion speed
    adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::LOW_SPEED); // change the sampling speed
    adc->adc1->enableInterrupts(callbacks);
    Serial.println("Done initializing ADCs");
}

uint32_t begin;

void ADC1_start() {
    uint8_t currentPin = pins[adcIndex];
    Serial.print( "Starting read for ");
    Serial.println(currentPin);
    Serial.print("time since prev start of read");
    uint32_t currentTime = micros();
    Serial.println(currentTime - begin);
    begin = micros();
    adc->adc1->startSingleRead(currentPin); // in callbacks.h

}
