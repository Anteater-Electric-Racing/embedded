// Anteater Electric Racing, 2025

#define THREAD_MAIN_STACK_SIZE 128
#define THREAD_MAIN_PRIORITY 1

#define WDT_CHECK_INTERVAL_MS 50
#define WDT_TIMEOUT_MS 1000

#include <Arduino.h>
#include <arduino_freertos.h>

#include "peripherals/peripherals.h"

// Struct to track sensor last update timestamps
static struct {
  volatile unsigned long brake;
  volatile unsigned long linear_pots;
  volatile unsigned long apps;
  volatile unsigned long imu;
  volatile unsigned long gps;
} sensorLastUpdate = {0,0,0,0,0};


void threadMain( void *pvParameters );
void initWatchdog();
void kickWatchdog();
bool checkSensors();

void initWatchdog(){
  // Clear Wide Mode Register 
  WDOG1_WMCR = 0;

  // Set timeout period (1 sec)
  WDOG1_WCR = WDOG_WCR_WT(0x3FF);

  WDOG1_WCR |= WDOG_WCR_WDE;  // Enable WDT
  WDOG1_WCR |= WDOG_WCR_SRE;  // System Reset Enable
  WDOG1_WCR &= ~WDOG_WCR_WDA; // Disable alt reset signal WDOG_B

  // Start timer
  kickWatchdog();
}

void kickWatchdog(){
  // First service key, pause
  __asm__ volatile ("nop");
  WDOG1_WSR = 0x5555;

  // Second, pause
  __asm__ volatile ("nop");
  WDOG1_WSR = 0xAAAA;
}

bool checkSensors(){
  // Start time
  unsigned long currentTime = millis();

  const unsigned long BRAKE_TIMEOUT = 200;
  const unsigned long LINEAR_POTS_TIMEOUT = 300;
  const unsigned long APPS_TIMEOUT = 200;
  const unsigned long IMU_TIMEOUT = 300;
  const unsigned long GPS_TIMEOUT = 1000;

  // Check sensors, return false if not working
  if (currentTime - sensorLastUpdate.brake > BRAKE_TIMEOUT){
    Serial.println("Brake sensor timeout");
    return false;
  }
  if (currentTime - sensorLastUpdate.linear_pots > LINEAR_POTS_TIMEOUT){
    Serial.println("Linear potentiometer sensors timeout");
    return false;
  }
  if (currentTime - sensorLastUpdate.apps > APPS_TIMEOUT){
    Serial.println("APPS timeout");
    return false;
  }
  if (currentTime - sensorLastUpdate.imu > IMU_TIMEOUT){
    Serial.println("IMU timeout");
    return false;
  }
  if (currentTime - sensorLastUpdate.gps > GPS_TIMEOUT){
    Serial.println("GPS timeout");
    return false;
  }

  // If all sensors are working:
  return true;
}

void setup() {
  initWatchdog();

  xTaskCreate(threadMain, "threadMain", THREAD_MAIN_STACK_SIZE, NULL, THREAD_MAIN_PRIORITY, NULL);
}

void threadMain( void *pvParameters ) {
  Peripherals_init();

  TickType_t xLastWakeTime;
  const TickType_t xCheckInterval = pdMS_TO_TICKS(WDT_CHECK_INTERVAL_MS);
  xLastWakeTime = xTaskGetTickCount();

  while (true) {
    // Main loop
    vTaskDelayUntil(&xLastWakeTime, xCheckInterval);

    // Only kick the watchdog if all sensors are working
    if (checkSensors()) {
        kickWatchdog();
    }
  }
}

void loop() {}

void updateBrakeSensor() {
    sensorLastUpdate.brake = millis();
}

void updateSuspensionSensor() {
    sensorLastUpdate.suspension = millis();
}

void updatePedalSensor() {
    sensorLastUpdate.pedal = millis();
}

void updateImuSensor() {
    sensorLastUpdate.imu = millis();
}

void updateGpsSensor() {
    sensorLastUpdate.gps = millis();
}