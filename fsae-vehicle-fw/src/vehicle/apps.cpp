// Anteater Electric Racing, 2025

#include <cmath>

class APPS {
    private: 
        float apps1, apps2;
        const float threshold = 0.1;
        bool APPS_fault = false;

        void checkAPPSFault() {
            if (abs(apps1 - apps2) > threshold) {
                APPS_fault = true;
                // motor_setFaultState();
                // fault.setFault(FAULT_APPS);
            }
        }

        void clearAPPSFault() {
            if (abs(apps1 - apps2) < threshold){
                APPS_fault = false;
                // fault.clearFault(FAULT_APPS);
                // send command to another module that checks if all faults are clear - fault.sendAllClear();
            }
        }

    public:
        void APPS_UpdateData(/*data*/) {
            // update data
            // apps1 = data.apps;
            // apps2 = data.apps2;

            // check if faulted then handle
            if (APPS_fault) {
                // attempt to clear fault
                clearAPPSFault();
            } else {
                // check if faulted
                checkAPPSFault();
            }

            // send to telemetry (or should this be in the adc loop?)
            // but need to send to telemetry that there is a apps fault
        }
};