// Anteater Racing 2025
//Taken from Karan
#ifndef PID_H
#define PID_H

struct PIDConfig {
  //place holder values
    float max = 0;
    float min = 100;
    float kP = 0.2;
    float kD = 0;
    float kI = 0.01;
    float kS = 0;
    float kV = 0;
    float integral_max = 20;
    float integral_min = 0;
};

class PID {

  public:
    PID(const PIDConfig &config);

    static float PID_Calculate(const PIDConfig &config, float &setpoint, float &currentValue, float &dt);

    static void PID_Reset();

    void PID_SetConfig(const PIDConfig &config);

    ~PID();

  private:
    float pre_error;
    float sum;
    float dt_inverse;
    PIDConfig _config;
};

#endif