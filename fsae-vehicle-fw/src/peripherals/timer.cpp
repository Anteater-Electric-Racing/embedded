#include "peripherals/timer.h"
#include "callbacks.h"

void Timer_StartADCScan(){
    readPinTimer.begin(StartADCScanCallback, 1000);
}
