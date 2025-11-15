// Anteater Electric Racing, 2025
#include "peripherals/adc.h"
#include "peripherals/can.h"
#include "peripherals/gpio.h"
#include "peripherals/peripherals.h"
#include "peripherals/tire_speed.h"

void Peripherals_Init() {
    ADC_Init();
    CAN_Init();
    Tire_Speed_Init();
}
