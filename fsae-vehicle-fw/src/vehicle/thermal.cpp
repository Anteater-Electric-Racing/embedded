#include <Arduino.h>
#include "vehicle/telemetry.h"

#define DUTY_CYCLE_MAX 255
#define ANALOG_WRITE_FREQUENCY 25000 // 25 kHz for Koolance
#define ANALOG_WRITE_RESOLUTION 8 // 8-bit resolution (0-255)

#define PUMP_PIN 12 // Define the PWM pin for the pump
#define FAN_PIN 7

#define TEMP_THRESHOLD 30 // Temperature threshold in degrees Celsius

void thermal_Init() {
  pinMode(PUMP_PIN, OUTPUT);
  analogWriteFrequency(PUMP_PIN, ANALOG_WRITE_FREQUENCY); // 25 kHz for Koolance
  analogWriteResolution(ANALOG_WRITE_RESOLUTION); // 0-255

  pinMode(FAN_PIN, OUTPUT);
  analogWriteFrequency(FAN_PIN, ANALOG_WRITE_FREQUENCY); // 25 kHz for Koolance
  analogWriteResolution(ANALOG_WRITE_RESOLUTION); // 0-255
}

void thermal_Update(uint32_t rawReading1, uint32_t rawReading2, uint32_t rawReading3, uint32_t rawReading4) {
  // Assuming rawReading1 and rawReading2 are the temperature readings from the sensors
  // Convert raw readings to temperature in degrees Celsius
    int32_t temp1 = (rawReading1);
    int32_t temp2 = (rawReading2);
    // int32_t temp3 = (rawReading3);
    // int32_t temp4 = (rawReading4);
    // Check if the MCU temperature exceeds the threshold
    if (temp1 > TEMP_THRESHOLD || temp2 > TEMP_THRESHOLD) {
        // If the MCU temperature exceeds the threshold, turn on the pump and fan
        analogWrite(PUMP_PIN, DUTY_CYCLE_MAX * 0.9); // Set pump to full duty cycle
        analogWrite(FAN_PIN, DUTY_CYCLE_MAX * 0.9); // Set fan to full duty cycle
    } else {
        // If the MCU temperature is below the threshold, turn off the pump and fan
        analogWrite(PUMP_PIN, 0); // Set pump to 0 duty cycle
        analogWrite(FAN_PIN, 0); // Set fan to 0 duty cycle
    }
    // Debugging output
    Serial.print("Temp1: ");
    Serial.print(temp1);
    Serial.print(" C | Temp2: ");
    Serial.print(temp2);
    Serial.println(" C");

}

