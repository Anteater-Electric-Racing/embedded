// Anteater Electric Racing, 2025

typedef struct {
    float bseReading1;
    float bseReading2;
} BSEData;

class BSE {
  private:
    BSEData _BSEdata;

    void checkAndHandleBSEFault();
    void verifySensorAgreement();

  public:
    BSE();
    ~BSE();

    void BSE_UpdateData(BSEData *data);
    float BSE_GetBSEReading();
};
