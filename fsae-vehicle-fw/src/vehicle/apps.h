// Anteater Electric Racing, 2025

typedef struct{
    float appsReading1;
    float appsReading2;
} APPSData;

class APPS {
    private:
        APPSData _APPSdata;

        void APPS::checkAndHandleAPPSFault();
        void APPS::checkAndHandlePlausibilityFault(BSE* bse);

    public:
        APPS();
        ~APPS();

        void APPS_UpdateData(APPSData* data);
        float APPS_GetAPPSReading();
};

typedef struct{
    float BSEReading1;
    float BSEReading2;
} BSEData;

class BSE {
    private:
        BSEData _BSEdata;

        void BSE::checkAndHandleBSEFault();
        void BSE::verifySensorAgreement();

    public:
        BSE();
        ~BSE();

        void BSE_UpdateData(APPSData* data);
        float BSE_GetBSEReading();
};
