// Anteater Racing 2025


#include "pid.h"

static float pre_error = 0;
static float sum = 0;
static float dt_inverse;

static float PID_Calculate(const PIDConfig &config, float &setpoint, float &currentValue, float &dt){

    //error calculation
    float error = setpoint - currentValue;
    sum += error * dt; //I

    //integral anti-wind-up
    if (sum > config.integral_max)
        sum = config.integral_max;
    else if (sum < config.integral_min)
        sum = config.integral_min;

    dt_inverse = 1 / dt;
    float derivative = (error - pre_error) * dt_inverse;  //D

    float feedback = (config.kP * error) + (config.kI  * sum) + (config.kD  * derivative); //PID output

    float feedforward =  (config.kV  * setpoint) + config.kS  * (setpoint > 0 ? 1 : -1); //FF output

    float output = feedback + feedforward;

    // set output range
    if( output > config.max )
        output = config.max;
    else if( output < config.min )
        output = config.min;

    pre_error = error;

    return output;
}
