// Anteater Racing 2025

#ifndef PID_H
#define PID_H

class PID{

    public:
        PID(double kP, double kI, double kD, double kS, double kV, double max, double min);

        double PID_Calculate(double setpoint, double dt, double currentvalue);

        void PID_SetFF(double kS, double kV);

        void PID_SetPID(double kP, double kI, double kD);

        ~PID();

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
