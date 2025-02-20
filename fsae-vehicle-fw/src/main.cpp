// Anteater Electric Racing, 2025

#define THREAD_MAIN_STACK_SIZE 128
#define THREAD_MAIN_PRIORITY 1

#define WDT_CHECK_INTERVAL_MS 500
#define WDT_TIMEOUT_MS 1000

#include <Arduino.h>
#include <arduino_freertos.h>

#include "peripherals/peripherals.h"
#include "peripherals/watchdog.h"

void threadMain( void *pvParameters );

void setup() {
  xTaskCreate(threadMain, "threadMain", THREAD_MAIN_STACK_SIZE, NULL, THREAD_MAIN_PRIORITY, NULL);
}

void threadMain( void *pvParameters ) {
  Peripherals_init();
  Watchdog_Init();

  while (true) {
    // Main loop
    Watchdog_Pet();
    vTaskDelay(pdMS_TO_TICKS(WDT_CHECK_INTERVAL_MS));
  }
}

void loop() {}
