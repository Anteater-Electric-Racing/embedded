// Anteater Electric Racing, 2025

#include "imu.h"

#include <arduino_freertos.h>
#define THREAD_IMU_STACK_SIZE 128
#define THREAD_IMU_PRIORITY 1

static struct IMUData imu_dat;

// Telemetry object is const pointer bc of mutual exclusion,
// so we just update local struct and telemetry pulls from it

void IMU_Read(void *pvParameters) {

    Wire.beginTransmission(IMU_ID);
    Wire.write(IMU_REG);
    Wire.endTransmission(false);
    Wire.requestFrom(IMU_ID, num_Reg * 2, true);

    while(true){
        // TODO:
        // Check if enough bytes available
        if (Wire.available() < num_Reg * 2) {

            for (int i = 0; i < num_Reg; i++) {
                int16_t data = Wire.read() << 8 | Wire.read();
                data = constrain(map(data, -1700, 1700, 0, 127));

                switch (i) {
                    case 0: imu_dat.accel_x = data; break;
                    case 1: imu_dat.accel_y = data; break;
                    case 2: imu_dat.accel_z = data; break;
                    case 3: imu_dat.temp = data; break;
                    case 4: imu_dat.gyro_x = data; break;
                    case 5: imu_dat.gyro_y = data; break;
                    case 6: imu_dat.gyro_z = data; break;
                }
            }

            Wire.requestFrom(IMU_ID, num_Reg * 2, true);
        }
    }
}

void IMU_Init() {
    Wire.setSDA(SDA_PIN);
    Wire.setSCL(SCL_PIN);
    Wire.begin();

    Wire.beginTransmission(IMU_ID);
    Wire.write(0);
    Wire.endTransmission(true);

    xTaskCreate(IMU_Read, "threadIMU", THREAD_IMU_STACK_SIZE, NULL,
                THREAD_IMU_PRIORITY, NULL);
}
