// Anteater Electric Racing, 2025

#include "peripherals/adc.h"
#include "peripherals/gpio.h"
#include "peripherals/peripherals.h"

void Peripherals_Init() {
    GPIO_Init();
    ADC0_Init();
    ADC1_Init();
    // init the rest of peripherals
}
