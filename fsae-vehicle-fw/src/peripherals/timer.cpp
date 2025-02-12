#include "peripherals/timer.h"
#include "callbacks.h"

void Timer_StartADCScan(){
    readPinTimer.begin(StartADCCompleteCallback, 1000);
}
