// Anteater Electric Racing, 2025

#ifndef FAULTS_H
#define FAULTS_H

typedef struct{
    bool overCurrent: 1;
    bool underVoltage: 1;
    bool overTemp: 1;
    bool APPS_fault: 1;
    bool BSE_fault: 1;
    bool APPS_brake_plausibility: 1;
} FaultMap;

typedef enum {
    FAULT_NONE,
    FAULT_OVER_CURRENT,
    FAULT_UNDER_VOLTAGE,
    FAULT_OVER_TEMP,
    FAULT_APPS,
    FAULT_BSE,
    FAULT_APPS_BRAKE_PLAUSIBILITY
} FaultType;

void Faults_Init();
void Faults_Set(FaultType fault);
FaultMap Faults_Get();
void Faults_Clear(FaultType fault);
void Faults_Check();
void Faults_Handle();

bool checkOverCurrent();
bool checkUnderVoltage();
bool checkOverTemp();
bool checkAPPS();
bool checkBSE();
bool checkABP();



#endif
