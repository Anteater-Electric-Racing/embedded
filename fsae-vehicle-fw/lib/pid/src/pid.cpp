// Anteater Racing 2025


#include <iostream>
#include <cmath>
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


