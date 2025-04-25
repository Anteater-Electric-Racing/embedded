// Anteater Electric Racing, 2025

#ifndef WATCHDOG_H
#define WATCHDOG_H

#include "Watchdog_t4.h"
#include <Arduino.h>

#define WDT_WINDOW_SEC 0.5
#define WDT_TIMEOUT_SEC 1.0
#define WDT_TRIGGER_SEC 0.8

void Watchdog_Init();

void Watchdog_Pet();

void myCallBack();

#endif
