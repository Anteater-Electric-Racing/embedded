// Anteater Electric Racing, 2025

#define THREAD_MAIN_STACK_SIZE 128
#define THREAD_MAIN_PRIORITY 1

#define WDT_CHECK_INTERVAL_MS 500
#define WDT_TIMEOUT_MS 1000

#include <Arduino.h>
#include <arduino_freertos.h>

#include "peripherals/peripherals.h"
#include "peripherals/watchdog.h"
#include "peripherals/watchdog.h"

void threadMain( void *pvParameters );
bool checkSensors();

bool checkSensors(){
  unsigned long currentTime = millis();

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
  return true;
}

void setup() {
  xTaskCreate(threadMain, "threadMain", THREAD_MAIN_STACK_SIZE, NULL, THREAD_MAIN_PRIORITY, NULL);
}

void threadMain( void *pvParameters ) {
  Peripherals_init();
  Watchdog_Init();

  while (true) {
    // Tells the computer to wait
    vTaskDelay(pdMS_TO_TICKS(WDT_CHECK_INTERVAL_MS));

    // Only pet the watchdog if all sensors are working
    if (checkSensors()) {
        Watchdog_Pet();
    }
}

void loop() {}

// Update sensor last updates
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