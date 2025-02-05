# KZ PID Library
Custom PID library with feedforward inputs. Used on KZ for traction and speed control.

## Usage
```http
  static float PID_Calculate(const PIDConfig &config
  float &setpoint, float &currentvalue, float &dt);

```
Takes the setpoint, current sensor value and the time delta and returns the desired output control value.

```http
  struct PIDConfig {
    float max;
    float min;
    float kP;
    float kD;
    float kI;
    float kS;
    float kV;
    float integral_max;
    float integral_min;
};

```
Custom struct to store PID config values. Used in PID_Calculate()




