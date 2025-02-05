#include "peripherals/timer.h"
#include "callbacks.h"

void startTimer(){
    int result = readPinTimer.begin(StartADCPinReadings, 1000);
}
