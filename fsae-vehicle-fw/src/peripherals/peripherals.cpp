// Anteater Electric Racing, 2025
#include "peripherals/adc.h"
#include "peripherals/gpio.h"
#include "peripherals/peripherals.h"
#include "vehicle/can.h"

void Peripherals_Init() {
    ADC_Init();
    CAN_Init();
}
