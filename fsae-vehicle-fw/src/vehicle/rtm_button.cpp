// Anteater Electric Racing, 2025

#define BUTTON_DEBOUNCE_MS 500

#include <Arduino.h>
#include <stdint.h>

#include "rtm_button.h"

static bool rtmState = false; // Latching state of RTM based on momentary button
                              // press. True - driving state, false - idle state
static uint32_t lastDebounceTime = 0;

void RTMButton_Update(bool rtmButton) {
    if (rtmButton == 1 && millis() - lastDebounceTime > BUTTON_DEBOUNCE_MS) {
        rtmState = !rtmState; // Toggle the state
        lastDebounceTime = millis();
    }
}

bool RTMButton_GetState() { return rtmState; }
