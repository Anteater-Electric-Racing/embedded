#include "apps.h"
#include "bse.h"
#include "wdt.h"

#include <Arduino.h>
#include <WDT_T4.h>
#include <arduino_freertos.h>

// Bitmask flag definition
static constexpr uint8_t WDT_BIT_BSE  = 0b01; 
static constexpr uint8_t WDT_BIT_APPS = 0b10;  

static constexpr uint8_t WDT_REQUIRED_MASK = 0b00; // 0b00 represents no flags


static volatile TickType_t bse_last_run_tick;
static volatile TickType_t apps_last_run_tick;

static WDT_T4<WDT1> WDT;

void WDT_Init() {
    TickType_t now = xTaskGetTickCount();
    bse_last_run_tick  = now;
    apps_last_run_tick = now;

    WDT_timings_t config;

    config.timeout  = 1.0;     // second before reset
    config.trigger  = 0.0;     
    config.callback = nullptr; 

    WDT.begin(config);

    Serial.println("Watchdog initialized (1 second timeout)");

}


void WDT_Update_Task(void* arg)
{
    // Change values to actual update periods
    static constexpr uint32_t BSE_EXPECTED_PERIOD_MS  = 100;
    static constexpr uint32_t APPS_EXPECTED_PERIOD_MS = 100;

    // Timeout = expected * 3 
    static constexpr uint32_t BSE_WDT_TIMEOUT_MS  = BSE_EXPECTED_PERIOD_MS  * 3;
    static constexpr uint32_t APPS_WDT_TIMEOUT_MS = APPS_EXPECTED_PERIOD_MS * 3;


    static constexpr uint32_t WDT_CHECK_PERIOD_MS = 100;

    TickType_t now;

    TickType_t bse_ageTicks;
    uint32_t bse_ageMs;

    TickType_t apps_ageTicks;
    uint32_t apps_ageMs;
    
    uint8_t mask;

    for (;;)
    {
        now = xTaskGetTickCount();

        bse_ageTicks = now - bse_last_run_tick;
        bse_ageMs = bse_ageTicks * portTICK_PERIOD_MS;

        apps_ageTicks = now - apps_last_run_tick;
        apps_ageMs = apps_ageTicks * portTICK_PERIOD_MS;

        mask = 0b00;

        if (bse_ageMs  > BSE_WDT_TIMEOUT_MS)  mask |= WDT_BIT_BSE; // x |= y  ==> x = x | y
        if (apps_ageMs > APPS_WDT_TIMEOUT_MS) mask |= WDT_BIT_APPS;

        // pet if 0b00 
        if (mask == WDT_REQUIRED_MASK)
        {
            WDT.feed();   // pet hardware watchdog   
        }

        vTaskDelay(pdMS_TO_TICKS(WDT_CHECK_PERIOD_MS)); // 100ms delay
    }
}