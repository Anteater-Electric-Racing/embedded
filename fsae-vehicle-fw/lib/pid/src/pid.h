// Anteater Racing 2025

#ifndef PID_H
#define PID_H

class PID{

    public:
        PID(double kP, double kI, double kD, double kS, double kV, double max, double min);

        double PID_Calculate(double setpoint, double dt);

        double PID_SetFF(double kS, double kV);

        double PID_SetPID(double kP, double kI, double kD);

        double PID_SetOutputRange(double max, double min);

    private:
        double _dt;
        double _max;
        double _min;
        double _kP;
        double _kD;
        double _kI;
        double _kS;
        double _kV;
        double _pre_error;
        double _sum;

};

#endif
