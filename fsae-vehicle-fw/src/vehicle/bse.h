// Anteater Electric Racing, 2025

typedef struct {
    float bseReading1;
    float bseReading2;
} BSEData;

BSEData bseData;

void BSE_Init();
void BSE_UpdateData(BSEData *data);
float BSE_GetBSEReading();
