// Anteater Electric Racing, 2025

#include "imu.h"

#include <arduino_freertos.h>
#define THREAD_IMU_STACK_SIZE 128
#define THREAD_IMU_PRIORITY 1

// Telemetry object is const pointer bc of mutual exclusion,
// so we just update local struct and telemetry pulls from it

void IMU_Read(void *pvParameters) {
    while(true){
        // TODO:
        // Check if enough bytes available
        // Yes Case:
            // Read bytes
            // Update local struct (defined in imu.h)
        // We don't check for no case
    }
}

void IMU_Init() {
    xTaskCreate(IMU_Read, "threadIMU", THREAD_IMU_STACK_SIZE, NULL,
                THREAD_IMU_PRIORITY, NULL);
}
