// Anteater Electric Racing, 2025

#include "peripherals/adc.h"
#include "peripherals/can.h"
#include "peripherals/gpio.h"
#include "peripherals/peripherals.h"
#include "peripherals/timer.h"

void Peripherals_Init() {
    GPIO_Init();
    ADC0_Init();
    ADC1_Init();
    CAN_Init();

    Timer_StartADCScan();
}
