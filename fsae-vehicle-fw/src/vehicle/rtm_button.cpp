// Anteater Electric Racing, 2025

#define BUTTON_DEBOUNCE_MS 100

#include <Arduino.h>
#include <stdint.h>

#include "rtm_button.h"

static bool rtmState = false; // Latching state of RTM based on momentary button press. True - driving state, false - idle state
static uint32_t lastDebounceTime = 0;
static uint32_t lastButtonState = 0; // Last state of the button

void RTMButton_Update(bool rtmButton) {
    if(rtmButton == 1 && lastButtonState == 0 && millis() - lastDebounceTime > BUTTON_DEBOUNCE_MS) { // Button pressed
        lastDebounceTime = millis();
        rtmState = !rtmState; // Toggle the state
    }
    lastButtonState = rtmButton; // Update last button state to pressed
}

bool RTMButton_GetState() {
    return rtmState;
}
