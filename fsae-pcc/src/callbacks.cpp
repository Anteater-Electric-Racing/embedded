#include "callbacks.h"

void StartPCCSensorCallback() {
    Serial.begin(9600);
    float pastSignal, newSignal, freq = 0;
    int count = 0;

    pastSignal = digitalRead(PIN1);
    while (true) {
        // Main loop code here
        // This is where you would typically handle tasks, read sensors, etc.
        Serial.println("Main loop running...");
        newSignal = digitalRead(PIN1);

        if (newSignal == pastSignal)
            count++;
        else if (count > 0) {
            float period = 2 * count;
            freq = 1/period;
            count = 0;
        }

        pastSignal = newSignal;
        Serial.print("frequency: ");
        Serial.println(freq);
    }
}
