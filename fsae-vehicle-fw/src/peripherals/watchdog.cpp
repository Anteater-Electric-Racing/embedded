// Anteater Electric Racing, 2025

#include "watchdog.h"
#include <Arduino.h>
#include <imxrt.h> // Uncomment if Teensy provides correct header
WDT_T4<WDT1> wdt; // Global WDT instance

bool checkSensors(){
  Serial.println("Testing watchdog without sensors");
  return true;

  /*unsigned long currentTime = millis();

  const unsigned long IMU_TIMEOUT = 300;
  const unsigned long GPS_TIMEOUT = 1000;

  if (currentTime - sensorLastUpdate.imu > IMU_TIMEOUT){
    Serial.println("IMU timeout");
    return false;
  }
  if (currentTime - sensorLastUpdate.gps > GPS_TIMEOUT){
    Serial.println("GPS timeout");
    return false;
  }

  // If all sensors are working:
  return true; */
}

void Watchdog_Init(){
  Serial.println("Initializing watchdog...");
  
  /*WDT_timings_t config;
  config.timeout = WDT_TIMEOUT_SEC;
  config.window = WDT_WINDOW_SEC; // must feed after 0.5s, before 1s
  config.trigger = WDT_TRIGGER_SEC; // warning trigger
  config.callback = myCallBack; // Full system reset (change back to nullptr) 

  wdt.begin(config);*/

  Serial.println("Watchdog initialized!");
}

void Watchdog_Pet(){
  wdt.feed();
  Serial.println("Watchdog has been pet.");
}

void myCallBack(){
  Serial.println("Feed wdt");
}
