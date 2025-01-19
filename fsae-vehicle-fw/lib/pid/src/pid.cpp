// Anteater Racing 2025

#include "pid.h"

PID::PID(const PIDConfig &config):
    _config(config),
    pre_error(0),
    sum(0)
    {}

PID::~PID() {}


void PID::PID_SetConfig(const PIDConfig &config){
    _config = config;
}

float PID::PID_Calculate(float &setpoint, float &dt, float &currentValue){

    //error calculation
    float error = setpoint - currentValue;

    sum += error * dt; //I
    float derivative = (error - pre_error) / dt;  //D

    float feedback = (_config.kP * error) + (_config.kI  * sum) + (_config.kD  * derivative); //PID output

    float feedforward =  (_config.kV  * setpoint) + _config.kS  * (setpoint > 0 ? 1 : -1); //FF output

    float output = feedback + feedforward;

    // set output range
    if( output > _config.max )
        output = _config.max;
    else if( output < _config.min )
        output = _config.min;

    pre_error = error;

    return output;
}


