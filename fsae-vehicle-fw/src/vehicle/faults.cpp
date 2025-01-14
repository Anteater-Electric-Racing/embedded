// Anteater Electric Racing, 2025

typedef struct{
    bool overCurrent: 1;
    bool underVoltage: 1;
    bool overTemp: 1;
    bool APPS_fault: 1;
    bool BSE_fault: 1;
    bool APPS_brake_plausibility: 1;
} FaultMap;
