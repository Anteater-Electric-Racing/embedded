// Anteater Electric Racing, 2025

#include <ADC.h>
#include "callbacks.h"
#include "peripherals/adc.h"

#define LOGIC_LEVEL_V 3.3
#define ADC_RESOLUTION 10
#define ADC1_MAX_VALUE (1 << ADC_RESOLUTION) - 1

void StartADCPinReadings() {
    uint8_t currentPin = pins[adcIndex];
    Serial.print( "Starting read for ");
    Serial.println(currentPin);
    Serial.print("time since prev start of read");
    uint32_t currentTime = micros();
    Serial.println(currentTime - begin);
    begin = micros();
    adc->adc1->startSingleRead(currentPin); // in callbacks.h

}

void ADCReadingCompleteCallback () {
    // read from pin
    // store pin reads
    // go to next pin

    uint16_t sensorRead = adc->adc1->readSingle();
    Serial.println( "In callback func");
    adcReads[adcIndex] = sensorRead*LOGIC_LEVEL_V/ADC1_MAX_VALUE; // Get value within 0V to 3.3V range
    Serial.print( "In ADCConversionCompleteCallback pin is ");
    Serial.println(adcPins[adcIndex]);
    Serial.print( "In ADCConversionCompleteCallback sensorRead is ");
    Serial.println(sensorRead);
    Serial.print( "In ADCReadingCompleteCallback value is ");
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
    adc->adc1->enableInterrupts(ADCReadingCompleteCallback);
    bool successfulStart = adc->adc1->startSingleRead(currentPin); // in callbacks.h
}

