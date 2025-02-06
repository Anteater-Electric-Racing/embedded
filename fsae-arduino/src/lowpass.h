#include <math.h>

#define ALPHA(cutoff, freq) {\ 
            float te = 1.0 / freq ;\
            float tau = 1.0 / (2*M_PI*cutoff) ;\
            (1.0 / (1.0 + tau/te)) ;\
           } 

#define LOW_PASS_FILTER(value, old, alpha) {\ 
            result = alpha*value + (1.0-alpha)*old ;\
           } 

#define FILTER(value, timestamp, dcutoff, mincutoff, last, freq, s, beta_) {\
            if (lasttime!=UndefinedTime && timestamp!=UndefinedTime)\
                freq = 1.0 / (timestamp-lasttime) ;\
            lasttime = timestamp ;\
            float dvalue = (value - last) * freq ;\ 
            float edvalue = LOW_PASS_FILTER(dvalue, s, ALPHA(dcutoff, freq)) ;\
            float cutoff = mincutoff + beta_*fabs(edvalue) ;\
            float arr[2] = {LOW_PASS_FILTER(value, last, ALPHA(cutoff)), edvalue} ;\
            (arr) ;\
            }