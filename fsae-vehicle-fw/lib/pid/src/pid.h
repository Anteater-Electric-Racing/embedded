// Anteater Racing 2025

#ifndef PID_H
#define PID_H

struct PIDConfig {
    float dt;
    float max;
    float min;
    float kP;
    float kD;
    float kI;
    float kS;
    float kV;
};

class PID {

  public:
    PID(const PIDConfig &config);

    float PID_Calculate(float &setpoint, float &dt, float &currentvalue);

    void PID_SetConfig(const PIDConfig &config);

    ~PID();

  private:
    float pre_error;
    float sum;
    PIDConfig _config;
};

#endif
