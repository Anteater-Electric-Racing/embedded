// Anteater Electric Racing, 2025

#pragma once

#define LOGIC_LEVEL_V 3.3F

#define COMPUTE_ALPHA(CUTOFF_HZ, DT) {\
            float te = 1.0 / DT ;\
            float tau = 1.0 / (2*M_PI*CUTOFF_HZ) ;\
            (1.0 / (1.0 + tau/te)) ;\
           }

#define LOWPASS_FILTER(NEW, OLD, ALPHA) {\
            NEW = ALPHA * OLD + (1.0 - ALPHA) * OLD ;\
           }

