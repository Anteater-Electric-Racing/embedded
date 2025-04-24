// Anteater Electric Racing, 2025

#include <Arduino.h>
#include "peripherals/gpio.h"

// rtm button: 4
// wheel speed 1: 2, wheel speed 2: 3
// using can1 and can2

void GPIO_Init() {
    pinMode(4, INPUT_PULLDOWN); // rtm button
    pinMode(2, INPUT); // wheel speed 1
    pinMode(3, INPUT); // wheel speed 2
}
