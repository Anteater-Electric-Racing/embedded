// Anteater Electric Racing, 2025

#include <Arduino.h>
#include "gpio.h"
#include "utils.h"

void gpioInit(void){
    pinMode(SHUTDOWN_CTRL_PIN, INPUT);
    pinMode(FREQ_ACCU_PIN, INPUT_PULLDOWN);
    pinMode(FREQ_TS_PIN, INPUT_PULLDOWN);
}
