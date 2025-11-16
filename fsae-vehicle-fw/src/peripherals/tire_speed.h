// Anteater Electric Racing, 2025

volatile uint32_t pulseCount = 0;
volatile uint32_t lastPulseMicros = 0;
volatile uint32_t pulsePeriodMicros = 0;

static const uint32_t NUM_TEETH = 30;    // example
static const float WHEEL_CIRCUMFERENCE_M = 2.0f; // example
#define HALL_PIN 5  // example

void Tire_Speed_Init();
void hallISR();
