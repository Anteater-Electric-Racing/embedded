// Anteater Electric Racing, 2025

#include <ADC.h>
#include "callbacks.h"
#include "peripherals/adc.h"

void callbacks () {
    // read from pin
    // store pin reads
    // go to next pin
    uint16_t sensorRead = adc->adc1->readSingle();
    adcReads[adcIndex] = sensorRead;
    ++adcIndex;
}

