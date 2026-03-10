#include <vehicle/motor.h>
#include <PID_v1.h>

void TractionControl(double FLSpeed, double FRSpeed, double BLSpeed, double BRSpeed, double& torqueRequest) {
    double kP = 0.1;
    double kI = 0;
    double kD = 0;
    double slipThreshold = 1;
    double error = 0;
    double drivenWheelAvg = 0;
    double frontWheelAvg = 0;
    double slip = 0;
    PID newPID(&slip, &torqueRequest, &slipThreshold, kP, kI, kD, DIRECT);
    newPID.SetMode(AUTOMATIC);
    while(1) {
        drivenWheelAvg = (BLSpeed + BRSpeed) / 2;
        frontWheelAvg = (FLSpeed + FRSpeed) / 2;
        slip = (drivenWheelAvg - frontWheelAvg) / frontWheelAvg;
        if (slip > slipThreshold) {
            newPID.Compute();
        }
    }
}

//link to lib: https://github.com/br3ttb/Arduino-PID-Library