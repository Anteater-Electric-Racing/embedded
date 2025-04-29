// Anteater Electric Racing, 2025

#include "watchdog.h"
#include <Arduino.h>
#include <imxrt.h> // Uncomment if Teensy provides correct header
WDT_T4<WDT1> wdt; // Global WDT instance


void Watchdog_Init(){
  Serial.println("Initializing watchdog...");

  WDT_timings_t config;
  config.timeout = WDT_TIMEOUT_SEC;
  config.window = WDT_WINDOW_SEC; // must feed after 0.5s, before 1s
  config.trigger = WDT_TRIGGER_SEC; // warning trigger
  config.callback = Watchdog_CallBack; // Full system reset (change back to nullptr)

  wdt.begin(config);

  Serial.println("Watchdog initialized!");
}

void Watchdog_Pet(){
  wdt.feed();
  Serial.println("Watchdog has been pet.");
}

void Watchdog_Reset(){
  Serial.println("Force reset");
  wdt.reset();
}

void Watchdog_CallBack(){
  Serial.println("WARNING: WATCHDOG NEEDS TO BE PET.");
}
