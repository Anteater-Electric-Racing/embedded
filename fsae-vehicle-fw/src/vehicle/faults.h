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
void Faults_SetFault(FaultType fault);
FaultMap* Faults_GetFaults(); 
void Faults_ClearFault(FaultType fault);
void Faults_CheckFaults();
void Faults_HandleFaults();
bool Faults_CheckAllClear();

bool checkOverCurrent();
bool checkUnderVoltage();
bool checkOverTemp();
bool checkAPPS();
bool checkBSE();
bool checkABP();



#endif
