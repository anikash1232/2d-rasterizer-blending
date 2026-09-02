/**
 *  Copyright 2016 Mike Reed
 */

#ifndef _bench_h_DEFINED
#define _bench_h_DEFINED

#include "../auto_register.h"
#include "../../include/GCanvas.h"
#include "../../include/GColor.h"
#include "../../include/GPoint.h"
#include "../../include/GRandom.h"
#include "../../include/GRect.h"
#include <memory>

class GBenchmark {
public:
    virtual ~GBenchmark() {}

    virtual const char* name() const = 0;
    virtual GISize size() const = 0;
    virtual void draw(GCanvas*) = 0;
};

using GBenchFact = GBenchmark*();

struct GBenchRec {
    GBenchFact* fFactory;
    int         fPA;
};

using BenchRegistrant = GRegistrant<GBenchRec>;
#define REGISTER_GBENCH(pa, code) \
    static BenchRegistrant G_MACRO_UNIQUE_NAME(gbench_proc)({ []() -> GBenchmark* { code }, pa })

///////////////////////

static inline GColor rand_color(GRandom& rand, bool forceOpaque = false) {
    GColor c { rand.nextF(), rand.nextF(), rand.nextF(), rand.nextF() };
    if (forceOpaque) {
        c.a = 1;
    }
    return c;
}

static inline GRect rand_rect(GRandom& rand, const GRect& bounds) {
    const float x = bounds.width();
    const float y = bounds.height();
    float tmp[4] {
        rand.nextF() * x - bounds.left, rand.nextF() * y - bounds.top,
        rand.nextF() * x - bounds.left, rand.nextF() * y - bounds.top
    };
    return GRect::XYWH(std::min(tmp[0], tmp[2]), std::min(tmp[1], tmp[3]),
                       std::max(tmp[0], tmp[2]), std::max(tmp[1], tmp[3]));
}

#endif
