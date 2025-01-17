// Anteater Racing 2025

#include "pid.h"

PID::PID(float kP, float kI, float kD, float kS, float kV, float max, float min):
    _kP(kP),
    _kI(kI),
    _kD(kD),
    _kS(kS),
    _kV(kV),
    _max(max),
    _min(min),
    _sum(0),
    _pre_error(0)
    {}

PID::~PID() {}


void PID::PID_SetPID(float kP, float kI, float kD){
    _kP = kP;
    _kI = kI;
    _kD = kD;
}

void PID::PID_SetFF(float kS, float kV){
    _kS = kS;
    _kV = kV;
}

float PID::PID_Calculate(float setpoint, float dt, float currentValue){

    //error calculation
    float error = setpoint - currentValue;

    float P = _kP * error; //P
    _sum += error * dt;     //I
    float derivative = (error - _pre_error) / dt;  //D

    float feedback = (_kP * error) + (_kI * _sum) + (_kD * derivative); //PID output

    float feedforward =  (_kV * setpoint) + _kS * (setpoint > 0 ? 1 : -1); //FF output

    float output = feedback + feedforward;

    // set output range
    if( output > _max )
        output = _max;
    else if( output < _min )
        output = _min;

    _pre_error = error;

    return output;
}


