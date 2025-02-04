// Anteater Electric Racing, 2025

#include "timer.h"
#include "peripherals/adc.h"
#include "peripherals/gpio.h"
#include "peripherals/peripherals.h"

void Peripherals_Init() {
    GPIO_Init();
    ADC1_init();
    Serial.print("initialized ADCs");
    // init the rest of peripherals
    // ADC1_start();
    startTimer();
}
