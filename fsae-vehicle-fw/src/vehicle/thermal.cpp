#include "vehicle/telemetry.h"
#include <Arduino.h>

#define DUTY_CYCLE_MAX 255
#define ANALOG_WRITE_FREQUENCY 25000 // 25 kHz for Koolance
#define FAN_WRITE_FREQ 500
#define ANALOG_WRITE_RESOLUTION 8 // 8-bit resolution (0-255)

#define PUMP1_PIN 29 // Define the PWM pin for the pump
#define PUMP2_PIN 28 // Define PWM pin for pump 2
#define FAN_PIN 7

#define PUMP_THRESHOLD 45 // Temperature threshold in degrees Celsius
#define FAN_THRESHOLD 50  // Temperature threshold in degrees Celsius

void thermal_forceOff() {
    analogWrite(PUMP1_PIN, 0);
    analogWrite(PUMP2_PIN, 0);
    analogWrite(FAN_PIN, DUTY_CYCLE_MAX);
}

void thermal_Init() {
    pinMode(PUMP1_PIN, OUTPUT);
    pinMode(PUMP2_PIN, OUTPUT);
    analogWriteFrequency(PUMP1_PIN,
                         ANALOG_WRITE_FREQUENCY);   // 25 kHz for Koolance
    analogWriteResolution(ANALOG_WRITE_RESOLUTION); // 0-255

    analogWriteFrequency(PUMP2_PIN,
                         ANALOG_WRITE_FREQUENCY);   // 25 kHz for Koolance
    analogWriteResolution(ANALOG_WRITE_RESOLUTION); // 0-255

    pinMode(FAN_PIN, OUTPUT);
    analogWriteFrequency(FAN_PIN,
                         FAN_WRITE_FREQ);           // 25 kHz for Koolance
    analogWriteResolution(ANALOG_WRITE_RESOLUTION); // 0-255

    // thermal_forceOff();
}

/*open loop  control */
/**
 * faults to add: (temp out of bounds??)
 *
 */

void thermal_forceOn() {
    analogWrite(PUMP1_PIN, DUTY_CYCLE_MAX * 0.9);
    analogWrite(PUMP2_PIN, DUTY_CYCLE_MAX * 0.9);
    analogWrite(FAN_PIN, DUTY_CYCLE_MAX * 0.1);
}

/*implement */
void thermal_MCULoop() {
    if ((MCU_GetMCU2Data()->mcuTemp <= 0) ||
        (MCU_GetMCU2Data()->motorTemp <= 0) ||
        (MCU_GetMCU2Data()->mcuTemp > 100) ||
        (MCU_GetMCU2Data()->motorTemp > 100)) {
        thermal_forceOn();
        return;
    }

    if (MCU_GetMCU2Data()->mcuTemp > PUMP_THRESHOLD ||
        MCU_GetMCU2Data()->motorTemp > PUMP_THRESHOLD) {
        analogWrite(PUMP1_PIN, DUTY_CYCLE_MAX * 0.9);
        analogWrite(PUMP2_PIN, DUTY_CYCLE_MAX * 0.9);
    } else if (MCU_GetMCU2Data()->mcuTemp < PUMP_THRESHOLD - 5 ||
               MCU_GetMCU2Data()->motorTemp < PUMP_THRESHOLD - 5) {
        analogWrite(PUMP1_PIN, 0);
        analogWrite(PUMP2_PIN, 0);
    }

    if (MCU_GetMCU2Data()->mcuTemp > FAN_THRESHOLD ||
        MCU_GetMCU2Data()->motorTemp > FAN_THRESHOLD) {
        analogWrite(FAN_PIN, DUTY_CYCLE_MAX * 0.1);
    } else if (MCU_GetMCU2Data()->mcuTemp < FAN_THRESHOLD - 5 ||
               MCU_GetMCU2Data()->motorTemp < FAN_THRESHOLD - 5) {
        analogWrite(FAN_PIN, DUTY_CYCLE_MAX);
    }
}
