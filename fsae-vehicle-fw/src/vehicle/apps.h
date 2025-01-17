// Anteater Electric Racing, 2025

typedef struct{
    float appsReading1;
    float appsReading2;
} APPSData;

class APPS {
    private:
        APPSData _APPSdata;
        bool APPS_fault;

        void checkAPPSFault();
        void clearAPPSFault();

    public:
        APPS();
        ~APPS();

        void APPS_UpdateData(APPSData* data);
        float APPS_GetAPPSReading();
};
