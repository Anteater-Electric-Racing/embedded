// Anteater Electric Racing, 2025

#pragma once

#define LOGIC_LEVEL_V 3.3F
#define TIME_STEP 0.001F // 1ms time step

#define COMPUTE_ALPHA(CUTOFF_HZ) \
    (1.0F / (1.0F + (1.0F / (2.0F * M_PI * CUTOFF_HZ)) / TIME_STEP))

#define LOWPASS_FILTER(NEW, OLD, ALPHA) \
    OLD = ALPHA * NEW + (1.0F - ALPHA) * OLD
