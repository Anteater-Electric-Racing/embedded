// Anteater Electric Racing, 2025

#define THREAD_MAIN_STACK_SIZE 128
#define THREAD_MAIN_PRIORITY 1

#include <Arduino.h>
#include <arduino_freertos.h>

#include "peripherals/peripherals.h"

void threadMain( void *pvParameters );

void setup() {
  xTaskCreate(threadMain, "threadMain", THREAD_MAIN_STACK_SIZE, NULL, THREAD_MAIN_PRIORITY, NULL);
}

void threadMain( void *pvParameters ) {
  Peripherals_init();
  while (true) {
    // Main loop
  }
}

void loop() {}
