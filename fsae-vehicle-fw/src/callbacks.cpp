// Anteater Electric Racing, 2025

#include <ADC.h>
#include "callbacks.h"
#include "peripherals/adc.h"

#define LOGIC_LEVEL_V 3.3
#define ADC_RESOLUTION 10
#define ADC1_MAX_VALUE (1 << ADC_RESOLUTION) - 1

void StartADCCompleteCallback() {
    noInterrupts();
    if (adcIndex != 0){ // means it was not reset at the end of the callback cycle: means callbacks did not complete
        _reboot_Teensyduino_();
    }
    adcIndex = 0;
    uint16_t startPin = adcPins[0];
    uint32_t currentTime = micros();
    adc->adc1->enableInterrupts(ADCConversionCompleteCallback);
    adc->adc1->startSingleRead(startPin); // in callbacks.h
    interrupts();
}

void ADCConversionCompleteCallback () {
    // read from pin
    // store pin reads
    // go to next pin
    noInterrupts();
    uint16_t sensorRead = adc->adc1->readSingle();
    adcReads[adcIndex] = sensorRead; //*LOGIC_LEVEL_V/ADC1_MAX_VALUE; // Get value within 0V to 3.3V range
    // TODO clear ADC data field if that's possible because otherwise there might be a bug with it reading two values close together too quickly?

    // increment to next sensor pin & start read
    ++adcIndex;
    if(adcIndex >= SENSOR_PIN_AMT){ // Do here so we don't start a read for an invalid pin
        adcIndex = 0;
    } else {
        uint16_t currentPin = adcPins[adcIndex];
        adc->adc1->enableInterrupts(ADCConversionCompleteCallback, adcIndex); // Priority gets lower as it's further in the index array
        adc->adc1->startSingleRead(currentPin); // in callbacks.h
    }
    interrupts();
}

