// Anteater Electric Racing, 2025
#include "peripherals/adc.h"
#include "peripherals/can.h"
#include "peripherals/gpio.h"
#include "peripherals/peripherals.h"

void Peripherals_Init() {
    ADC_Init();
    ADC_Begin();
    CAN_Init();
}
