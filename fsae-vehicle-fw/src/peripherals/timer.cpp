#include "peripherals/timer.h"
#include "callbacks.h"

void Timer_StartADCScan(){
    readPinTimer.begin(StartADCPinReadings, 1000);
}
