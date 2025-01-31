// Anteater Electric Racing, 2025

#include <ADC.h>
#include "callbacks.h"
#include "peripherals/adc.h"

void callbacks () {
    // read from pin
    // store pin reads
    // go to next pin
    // Serial.print("Timer frequency is ");
    // Serial.println(adc->adc1->getQuadTimerFrequency());
    if(adcIndex == SENSOR_PIN_AMT){ // Maybe do later to avoid having out of bounds
        adcIndex = 0;
        Serial.println( "In callbacks: got to end of pins ");
        // Serial.println(std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count());
        return;
    }

    uint16_t sensorRead = adc->adc1->readSingle();
    adcReads[adcIndex] = sensorRead;
    Serial.print( "In callbacks pin is ");
    Serial.println(pins[adcIndex]);
    Serial.print( "In callbacks sensorRead is ");
    Serial.println(sensorRead);

    // increment to next sensor pin & start read
    ++adcIndex;

    // Serial.print( "In callbacks: starting read for ");
    uint8_t currentPin = pins[adcIndex];
    // Serial.println(adcIndex);
    adc->adc1->enableInterrupts(callbacks);
    bool successfulStart = adc->adc1->startSingleRead(currentPin); // in callbacks.h
    // adc->adc1->startTimer(1000);
    // Serial.print( "In callbacks: successful start of read is ");
    // Serial.println(successfulStart);
    // adc->adc1->startTimer(1000); // every 0.001 seconds. starts single read every time period
}

