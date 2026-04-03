#include <Arduino.h>
#include <Watchdog_t4.h>
#include <arduino_freertos.h>

#include "peripherals/wdt.h"
#include "utils/utils.h"
#include "vehicle/apps.h"
#include "vehicle/bse.h"

// Global watchdog tick tracking variables
TickType_t bse_last_run_tick = 0;
TickType_t apps_last_run_tick = 0;

// Bitmask flag definition
static uint8_t WDT_BIT_BSE = 0b01;
static uint8_t WDT_BIT_APPS = 0b10;

static uint8_t WDT_REQUIRED_MASK = 0b00; // 0b00 represents no flags

static constexpr uint32_t WDT_CHECK_PERIOD_MS = 100;

static WDT_T4<WDT1> WDT;

void WDT_Init() {
    TickType_t now = xTaskGetTickCount();
    bse_last_run_tick = now;
    apps_last_run_tick = now;

    WDT_timings_t config;

    config.timeout = 1.0; // second before reset
    config.trigger = 0.0;
    config.callback = nullptr;

    WDT.begin(config);

    Serial.println("Watchdog initialized (1 second timeout)");
}

void WDT_Update_Task() {
    TickType_t now;

    TickType_t bse_ageTicks;
    uint32_t bse_ageMs;

    TickType_t apps_ageTicks;
    uint32_t apps_ageMs;

    uint8_t mask;

    for (;;) {
        now = xTaskGetTickCount();

        bse_ageTicks = now - bse_last_run_tick;
        bse_ageMs = bse_ageTicks * portTICK_PERIOD_MS;

        apps_ageTicks = now - apps_last_run_tick;
        apps_ageMs = apps_ageTicks * portTICK_PERIOD_MS;

        mask = 0b00;

        // Fault time are both 100 ms
        if (bse_ageMs > BSE_FAULT_TIME_THRESHOLD_MS) {
            mask |= WDT_BIT_BSE; // x |= y ==> x = x | y
        }
        if (apps_ageMs > APPS_FAULT_TIME_THRESHOLD_MS) {
            mask |= WDT_BIT_APPS;
        }

        // pet if 0b00
        if (mask == WDT_REQUIRED_MASK) {
            WDT.feed(); // pet hardware watchdog
        } else if (mask == WDT_BIT_BSE) {
            Serial.println("WDT: BSE update overdue");
        } else if (mask == WDT_BIT_APPS) {
            Serial.println("WDT: APPS update overdue");
        } else if (mask == (WDT_BIT_BSE | WDT_BIT_APPS)) { // mask = 0b11
            Serial.println("WDT: BSE and APPS updates overdue");
        }

        vTaskDelay(pdMS_TO_TICKS(WDT_CHECK_PERIOD_MS)); // 100ms delay
    }
}
