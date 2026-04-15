#pragma once

#include <arduino_freertos.h>

extern TickType_t adc_last_run_tick;
extern TickType_t main_last_run_tick;
extern TickType_t motor_last_run_tick;
extern TickType_t telemetry_last_run_tick;

void WDT_Init();
void WDT_Update_Task(void *pvParameters);
