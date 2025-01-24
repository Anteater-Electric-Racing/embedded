// Anteater Electric Racing, 2025

#include <ADC.h>
#include "callbacks.h"
#include "peripherals/adc.h"

void callbacks () {
    // read from pin
    // store pin reads
    // go to next pin
    Serial.print( "In callbacks on pin ");
    Serial.println(pins[adcIndex]);
    uint16_t sensorRead = adc->adc1->readSingle();
    adcReads[adcIndex] = sensorRead;
    Serial.print( "In callbacks sensorRead is ");
    Serial.println(sensorRead);

    // increment to next sensor pin
    if(adcIndex == SENSOR_PIN_AMT - 1){
        adcIndex = 0;
        Serial.print( "In callbacks: got to end of pins ");

    } else {
        ++adcIndex;
    }

    Serial.print( "In callbacks: starting read for ");
    uint8_t currentPin = pins[adcIndex];
    Serial.println(adcIndex);
    bool successfulStart = adc->adc1->startSingleRead(currentPin); // in callbacks.h
    Serial.print( "In callbacks: successful start of read is ");
    Serial.println(successfulStart);
    // adc->adc1->startTimer(1000); // every 0.001 seconds. starts single read every time period
}

