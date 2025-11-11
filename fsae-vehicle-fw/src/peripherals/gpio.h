// Anteater Electric Racing, 2025

#pragma once

#include <stdint.h>

void GPIO_Init();

// Read the current logical level of a GPIO pin.
// Returns 0 for LOW, non-zero for HIGH.
int GPIO_Read(int pin);
