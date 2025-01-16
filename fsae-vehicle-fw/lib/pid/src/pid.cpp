// Anteater Racing 2025


#include "pid.h"

PID::PID(double kP, double kI, double kD, double kS, double kV, double max, double min):
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


void PID::PID_SetPID(double kP, double kI, double kD){
    _kP = kP;
    _kI = kI;
    _kD = kD;
}

void PID::PID_SetFF(double kS, double kV){
    _kS = kS;
    _kV = kV;
}

double PID::PID_Calculate(double setpoint, double dt, double currentValue){

    //error calculation
    double error = setpoint - currentValue;

    double P = _kP * error; //P
    _sum += error * dt;     //I
    double derivative = (error - _pre_error) / dt;  //D

    double feedback = (_kP * error) + (_kI * _sum) + (_kD * derivative); //PID output

    double feedforward =  (_kV * setpoint) + _kS * (setpoint > 0 ? 1 : -1); //FF output

    double output = feedback + feedforward;

    // set output range
    if( output > _max )
        output = _max;
    else if( output < _min )
        output = _min;

    _pre_error = error;

    return output;
}


