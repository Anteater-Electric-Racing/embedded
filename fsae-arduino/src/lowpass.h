#include <math.h>

#define Alpha(cutoff, freq) {\ 
            float te = 1.0 / freq ;\
            float tau = 1.0 / (2*M_PI*cutoff) ;\
            (1.0 / (1.0 + tau/te)) ;\
           } 

#define LowPass(value, a, s, init) {\ 
            if (init)\
                result = a*value + (1.0-a)*s ;\
            else {\
                result = value ;\
                (init) = true ;\
            }\
            (result) ;\
           } 

#define Filter(value, timestamp, dcutoff, mincutoff, init1, init2, last, freq, s, beta_) {\
            if (lasttime!=UndefinedTime && timestamp!=UndefinedTime)\
                freq = 1.0 / (timestamp-lasttime) ;\
            lasttime = timestamp ;\
            float dvalue = init1 ?  0.0 : (value - last) * freq ;\ 
            float edvalue = LowPass(dvalue, Alpha(dcutoff, freq), s, init2) ;\
            float cutoff = mincutoff + beta_*fabs(edvalue) ;\
            float arr[2] = {LowPass(value, alpha(cutoff), last, init1), edvalue} ;\
            (arr) ;\
            }