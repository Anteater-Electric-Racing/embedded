#include "peripherals/timer.h"
#include "peripherals/peripherals.h"

void Peripherals_Init() {
    Timer_StartPCCSampler();
}
