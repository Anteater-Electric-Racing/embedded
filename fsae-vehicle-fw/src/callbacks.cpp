// Anteater Electric Racing, 2025

#include <ADC.h>
#include "callbacks.h"
#include "peripherals/adc.h"

#define LOGIC_LEVEL_V 3.3
#define ADC_RESOLUTION 10
#define ADC1_MAX_VALUE (1 << ADC_RESOLUTION) - 1

void StartADCPinReadings() {
    uint16_t startPin = adcPins[0];
    uint32_t currentTime = micros();
    adc->adc1->enableInterrupts(ADCConversionCompleteCallback);
    adc->adc1->startSingleRead(startPin); // in callbacks.h
}

void ADCConversionCompleteCallback () {
    // read from pin
    // store pin reads
    // go to next pin

    uint16_t sensorRead = adc->adc1->readSingle();
    adcReads[adcIndex] = sensorRead*LOGIC_LEVEL_V/ADC1_MAX_VALUE; // Get value within 0V to 3.3V range

    // increment to next sensor pin & start read
    ++adcIndex;
    if(adcIndex == SENSOR_PIN_AMT){ // Do here so we don't start a read for an invalid pin
        adcIndex = 0;
        return;
    }
    uint16_t currentPin = adcPins[adcIndex];
    adc->adc1->enableInterrupts(ADCConversionCompleteCallback);
    adc->adc1->startSingleRead(currentPin); // in callbacks.h
}

