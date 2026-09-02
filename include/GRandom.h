/**
 *  Copyright 2015 Mike Reed
 */

#ifndef GRandom_DEFINED
#define GRandom_DEFINED

#include "GTypes.h"

class GRandom {
public:
    GRandom(uint32_t seed = 0) : fSeed(seed) {}

    uint32_t nextU() {
        fSeed = fSeed * 1664525 + 1013904223;   // numerical recipies
        return fSeed;
    }

    float nextF() {
        return ((float)(this->nextU() & 0xFFFFFF)) / ((float)(1 << 24));
    }

private:
    uint32_t fSeed;
};

#endif

