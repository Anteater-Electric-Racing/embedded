// Anteater Electric Racing, 2025

#include "bse.h"

typedef struct {
    float appsReading1;
    float appsReading2;
} APPSData;

class APPS {
  private:
    APPSData _APPSdata;

    void checkAndHandleAPPSFault();
    void checkAndHandlePlausibilityFault(BSE *bse);

  public:
    APPS();
    ~APPS();

    void APPS_UpdateData(APPSData *data);
    float APPS_GetAPPSReading();
};
