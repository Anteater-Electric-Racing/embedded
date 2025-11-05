// Anteater Electric Racing, 2025

// #include "imu.h"

#include <arduino_freertos.h>
#include "../utils/utils.h"

template <typename T>
T constrain(T val, T minVal, T maxVal) {
    if (val < minVal) return minVal;
    if (val > maxVal) return maxVal;
    return val;
}
#define SENSOR_PIN_AMT 3

static struct sensor_t
{
    /* data */
    float shockTravel_mm;
    float steeringAngle_deg;
    float steeringTorque_Nm;
} sensorData;


uint16_t sensorPins[SENSOR_PIN_AMT] = {A0, A1, A2};
//uint16_t sensorIndex;                  // ***Im not entirely sure what pins, and how pins are mapped to what sensors for these 3 lines
uint16_t sensorReads[SENSOR_PIN_AMT];

float shockTravel_mm = 0.0f;
float steeringAngle_deg = 0.0f;
float steeringTorque_Nm = 0.0f;

const float ADC_VOLTAGE = 5.0f; 
const float ADC_RESOLUTION = 1023;   //***Unsure of these two, but searched them up to find these values

const float SHOCK_TRAVEL_MAX_MM = 50.0f;      // Placeholder: 
const float STEERING_ANGLE_MAX_DEG = 345.0f;  // Based on potentiometer electrical range on the website
const float TORQUE_SENSOR_MAX_NM = 10.0f;     // Also Placeholder; adjust after calibration

void threadSensor(void *pvParameters) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    while(true) {
        // Read sensors here
        for (int i = 0; i < SENSOR_PIN_AMT; i++) {
            sensorReads[i] = analogRead(sensorPins[i]);
        }
        //Converts to the bit-RES values from raw voltage, which supposedly teensy can't read?
        float shockVoltage   = (sensorReads[0] / ADC_RESOLUTION) * ADC_VOLTAGE;
        float steerAngleVolt = (sensorReads[1] / ADC_RESOLUTION) * ADC_VOLTAGE; 
        float torqueVoltage  = (sensorReads[2] / ADC_RESOLUTION) * ADC_VOLTAGE;
        //Converts those bit-RES values into the actual human-read values;
        sensorData.shockTravel_mm = constrain((shockVoltage / 5.0f) * SHOCK_TRAVEL_MAX_MM, 0.0f, SHOCK_TRAVEL_MAX_MM);
        sensorData.steeringAngle_deg = constrain((steerAngleVolt / 5.0f) * STEERING_ANGLE_MAX_DEG, 0.0f, STEERING_ANGLE_MAX_DEG);
        sensorData.steeringTorque_Nm = constrain((torqueVoltage / 5.0f) * TORQUE_SENSOR_MAX_NM, 0.0f, TORQUE_SENSOR_MAX_NM);

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(10)); // 100 Hz

    }
}

void Sensor_Init() {
    //Initialize sensor pins
    for (int i = 0; i < SENSOR_PIN_AMT; i++) {
        pinMode(sensorPins[i], INPUT);
    }

    xTaskCreate(threadSensor, "SensorThread", 128, NULL, 2, NULL);  //Im not entirely sure what the optimal stack size is, this might overflow easily
}

float Get_ShockTravel_mm() {
    return sensorData.shockTravel_mm;
}

float Get_SteeringAngle_deg() {
    return sensorData.steeringAngle_deg;
}

float Get_SteeringTorque_Nm() {
    return sensorData.steeringTorque_Nm;
}