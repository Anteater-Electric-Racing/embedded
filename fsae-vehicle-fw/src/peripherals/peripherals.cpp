// Anteater Electric Racing, 2025

#include "peripherals/timer.h"
#include "peripherals/adc.h"
#include "peripherals/gpio.h"
#include "peripherals/peripherals.h"

void Peripherals_Init() {
    GPIO_Init();
    ADC1_Init();
    Timer_StartADCScan();
}
