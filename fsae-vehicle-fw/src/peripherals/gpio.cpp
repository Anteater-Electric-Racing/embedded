// Anteater Electric Racing, 2025

#include "peripherals/gpio.h"
#include "utils/utils.h"
#include <Arduino.h>

// rtm button: 4
// wheel speed 1: 2, wheel speed 2: 3
// using can1 and can2

void GPIO_Init() {
    pinMode(2, INPUT_PULLUP);                // wheel speed 1
    pinMode(3, INPUT_PULLUP);                // wheel speed 2
    pinMode(RTM_BUTTON_PIN, INPUT_PULLDOWN); // rtm button
    pinMode(9, OUTPUT);
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

void GPIO_SetHigh(int pin) {
#ifdef digitalReadFast
    return digitalReadFast(pin);
#else
    digitalWrite(pin, HIGH);
#endif
}

void GPIO_SetLow(int pin) {
#ifdef digitalReadFast
    return digitalReadFast(pin);
#else
    digitalWrite(pin, LOW);
#endif
}
