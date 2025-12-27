// Anteater Electric Racing, 2025

#include "peripherals/gpio.h"
#include <Arduino.h>

// rtm button: 4
// wheel speed 1: 2, wheel speed 2: 3
// using can1 and can2

void GPIO_Init() {
    pinMode(2, INPUT_PULLUP);   // wheel speed 1
    pinMode(3, INPUT_PULLUP);   // wheel speed 2
    pinMode(4, INPUT_PULLDOWN); // rtm button
}

// Read the current logical level of a GPIO pin.
// Uses digitalReadFast when available on the platform for better performance.
int GPIO_Read(int pin) {
#ifdef digitalReadFast
    return digitalReadFast(pin);
#else
    return digitalRead(pin);
#endif
}
