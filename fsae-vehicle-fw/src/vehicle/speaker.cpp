// Anteater Electric Racing, 2026

#include "speaker.h"

void Speaker_Init() { pinMode(speakerPin, OUTPUT); }

void Speaker_Play() {
    // play a 440Hz tone for 500ms
    tone(speakerPin, 440);
    delay(500);

    // pause for 250ms
    noTone(speakerPin);
    delay(250);

    // play an 880Hz tone for 500ms
    tone(speakerPin, 880);
    delay(500);

    // pause for 2s
    noTone(speakerPin);
    delay(2000);
}
