// Anteater Electric Racing, 2025

#define THREAD_MAIN_STACK_SIZE 128
#define THREAD_MAIN_PRIORITY 1


#include <Arduino.h>
#include <arduino_freertos.h>

#include "peripherals/peripherals.h"
#include "peripherals/watchdog.h"


void threadMain( void *pvParameters );

void setup() {
  
  
  
  xTaskCreate(threadMain, "threadMain", THREAD_MAIN_STACK_SIZE, NULL, THREAD_MAIN_PRIORITY, NULL);
  vTaskStartScheduler();
}

void threadMain( void *pvParameters ) {
  Serial.begin(9600);
  // Peripherals_init();
  // Watchdog_Init();

  //delay(500);
  // while (!Serial && millis() < 4000); // test
  Serial.println("System starting..."); // Testing

  while (true) {
    // Tells the computer to wait
    vTaskDelay(1000);

    // Only pet the watchdog if all sensors are working
    if (checkSensors()) {
        // Watchdog_Pet();
    }
  }
}

void loop() {}

// Update sensor last updates
/*
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
*/