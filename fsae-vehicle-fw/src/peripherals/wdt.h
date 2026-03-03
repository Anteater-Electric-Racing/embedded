#pragma once

#include <arduino_freertos.h>

extern TickType_t bse_last_run_tick;
extern TickType_t apps_last_run_tick;

void WDT_Init();
void WDT_Task();
