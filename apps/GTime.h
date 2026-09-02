/**
 *  Copyright 2026 Mike Reed
 */

#ifndef GTime_DEFINED
#define GTime_DEFINED

#include "../include/GTypes.h"

#include <sys/time.h>

using GSecs = double;

class GTime {
public:
    static GSecs Secs() {
        struct timeval tv;

        if (gettimeofday(&tv, NULL)) {
            return 0;
        } else {
            return tv.tv_sec + tv.tv_usec / (1000.0 * 1000);
        }
    }
};

#endif
