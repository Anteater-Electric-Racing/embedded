// Anteater Electric Racing, 2025

#include "watchdog.h"
#include <Arduino.h>
// #include <imxrt.h> Uncomment if Teensy provides correct header

void initWatchdog(){
    // Clear Wide Mode Register 
    WDOG1_WMCR = 0;
  
    // Set timeout period (1 sec)
    WDOG1_WCR = WDOG_WCR_WT(0x3FF);
  
    WDOG1_WCR |= WDOG_WCR_WDE;  // Enable WDT
    WDOG1_WCR |= WDOG_WCR_SRE;  // System Reset Enable
    WDOG1_WCR &= ~WDOG_WCR_WDA; // Disable alt reset signal WDOG_B
  
    // Start timer
    Watchdog_Pet();
}

void Watchdog_Pet(){
  // First service key, pause
  __asm__ volatile ("nop");
  WDOG1_WSR = 0x5555;

  // Second, pause
  __asm__ volatile ("nop");
  WDOG1_WSR = 0xAAAA;
}
