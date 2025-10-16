#include <Arduino.h>
#include <algorithm>
#include <arduino_freertos.h>

#include "../utils/pid.h"

static long currentMillis, prevMillis = 0;

void LaunchControl_Init() {

}

void LaunchControl_Update(float velocity, float accel, float dt) {
    // Update launch control logic here
    slip = (velocity - refWheelSpeed) / refWheelSpeed;
    PID::PID_Calculate(, 0.1, slip, dt);
}