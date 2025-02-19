// Anteater Electric Racing, 2025

typedef struct{
    float appsReading1;
    float appsReading2;
} APPSData;

typedef struct{
    float BSEReading1;
    float BSEReading2;
} BSEData;


class APPS {
    private:
        APPSData _APPSdata;

        void checkAndHandleAPPSFault();
        void checkAndHandlePlausibilityFault(BSEData* bse);

    public:
        APPS();
        ~APPS();

        void APPS_UpdateData(APPSData* data);
        float APPS_GetAPPSReading();
};


class BSE {
    private:
        BSEData _BSEdata;

        void checkAndHandleBSEFault();
        void verifySensorAgreement();

    public:
        BSE();
        ~BSE();

        void BSE_UpdateData(APPSData* data);
        float BSE_GetBSEReading();
};
