// Anteater Electric Racing, 2025

#include <ADC.h>
#include "callbacks.h"
#include "peripherals/adc.h"


void callbacks () {
    // read from pin
    // store pin reads
    // go to next pin

    uint16_t sensorRead = adc->adc1->readSingle();
    adcReads[adcIndex] = sensorRead*3.3/adc->adc0->getMaxValue(); // Changing to this formula for now to see if it helps w apps fault sensorRead;
    Serial.print( "In callbacks pin is ");
    Serial.println(pins[adcIndex]);
    Serial.print( "In callbacks sensorRead is ");
    Serial.println(sensorRead);
    Serial.print( "In callbacks value is ");
    Serial.println(sensorRead*3.3/adc->adc0->getMaxValue(), DEC);

    // increment to next sensor pin & start read
    ++adcIndex;
    if(adcIndex == SENSOR_PIN_AMT){ // Do here so we don't start a read for an invalid pin
        adcIndex = 0;
        Serial.print("end of loop thinks value of begin is ");
        Serial.println(begin);
        Serial.print("duration of all reads: ");
        uint32_t end = micros();
        Serial.println(end - begin);
        return;
    }
    uint16_t currentPin = pins[adcIndex];
    adc->adc1->enableInterrupts(callbacks);
    bool successfulStart = adc->adc1->startSingleRead(currentPin); // in callbacks.h
}

