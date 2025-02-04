#include "timer.h"
#include "peripherals/adc.h"

void startTimer(){
    int result = readPinTimer.begin(ADC1_start, 1000);
    Serial.println(result);
}
