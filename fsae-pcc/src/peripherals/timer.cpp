#include "peripherals/timer.h"
#include "callbacks.h"

void Timer_StartPCCSampler() {
    readPinTimer.begin(StartPCCSensorCallback, 1000);
}
