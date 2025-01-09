// Anteater Electric Racing, 2025

#include "peripherals/adc.h"
#include "peripherals/gpio.h"
#include "peripherals/peripherals.h"

void Peripherals_init() {
    GPIO_init();
    ADC0_init();
    ADC1_init();
    // init the rest of peripherals
}
