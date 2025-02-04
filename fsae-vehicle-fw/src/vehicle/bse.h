// Anteater Electric Racing, 2025

typedef struct{
    float appsReading1;
    float appsReading2;
} APPSData;

typedef struct{
    float bseReading1;
    float bseReading2;
} BSEData;

typedef struct {
    APPSData appsData;
    BSEData bseData;
} APPSBSEData;

void checkAndHandleAPPSFault();
void checkAndHandlePlausibilityFault();
void APPS_UpdateData(APPSData* appsData);
float APPS_GetAPPSReading();

void checkAndHandleBSEFault();
void verifySensorAgreement();
void BSE_UpdateData(BSEData* bseData);
float BSE_GetBSEReading();

