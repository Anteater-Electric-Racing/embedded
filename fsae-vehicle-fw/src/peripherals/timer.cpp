#include "peripherals/timer.h"
#include "callbacks.h"

void Timer_StartADCScan(){
    int result = readPinTimer.begin(StartADCPinReadings, 1000);
}
