// Anteater Electric Racing, 2025

#ifndef IMU_H
#define IMU_H

#include <Wire.h>

#define SDA_PIN 25
#define SCL_PIN 24

#define IMU_ID 0x68
#define IMU_REG 0x3B
#define num_Reg 7

// TODO: Define IMU Struct
typedef struct {
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t temp;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
} IMUData;

void IMU_Read(void *pvParameters);
void IMU_Init();

#endif // IMU_H
