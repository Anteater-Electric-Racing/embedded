// Anteater Electric Racing, 2025
#include "peripherals/peripherals.h"
#include "peripherals/adc.h"
#include "peripherals/can.h"
#include "peripherals/gpio.h"

void Peripherals_Init() {
    ADC_Init();
    CAN_Init();
}
