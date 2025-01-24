// Anteater Electric Racing, 2025

#include "peripherals/adc.h"
#include "peripherals/gpio.h"
#include "peripherals/peripherals.h"

void Peripherals_init() {
    GPIO_init();
    ADC1_init();
    Serial.print("initialized ADCs");
    // init the rest of peripherals
    ADC1_start();
}
