/**
*  Copyright 2015 Mike Reed
*/

#include "bench.h"

class ClearBench : public GBenchmark {
    enum { W = 200, H = 200 };
public:
    ClearBench() {}

    const char* name() const override { return "clear"; }
    GISize size() const override { return { W, H }; }
    void draw(GCanvas* canvas) override {
        const int N = 500;
        for (int i = 0; i < N; ++i) {
            canvas->clear({1, 1, 0, 0.5f});
        }
    }
};

class RectsBench : public GBenchmark {
    enum { W = 200, H = 200 };
    const bool fForceOpaque, fUseLines;
public:
    RectsBench(bool forceOpaque, bool useLines) : fForceOpaque(forceOpaque), fUseLines(useLines) {}

    const char* name() const override {
        if (fUseLines) {
            return fForceOpaque ? "lines_opaque" : "lines_blend";
        } else {
            return fForceOpaque ? "rects_opaque" : "rects_blend";
        }
    }
    GISize size() const override { return { W, H }; }
    void draw(GCanvas* canvas) override {
        const int N = 500;
        const GRect bounds = GRect::LTRB(-10, -10, W + 10, H + 10);
        GRandom rand;
        for (int i = 0; i < N; ++i) {
            const GColor color = rand_color(rand, fForceOpaque);
            const GRect rect = rand_rect(rand, bounds);
            if (fUseLines) {
                if (i & 1) {
                    for (float y = rect.top + 0.5f; y < rect.bottom; y += 1) {
                        canvas->hairLine({rect.left, y}, {rect.right, y}, color);
                    }
                } else {
                    for (float x = rect.left + 0.5f; x < rect.right; x += 1) {
                        canvas->hairLine({x, rect.top}, {x, rect.bottom}, color);
                    }
                }
            } else {
                canvas->fillRect(rect, color);
            }
        }
    }
};

class SingleRectBench : public GBenchmark {
    const GISize    fSize;
    const GRect     fRect;
    const char*     fName;
public:
    SingleRectBench(GISize size, GRect r, const char* name) : fSize(size), fRect(r), fName(name) {}

    const char* name() const override { return fName; }
    GISize size() const override { return fSize; }
    void draw(GCanvas* canvas) override {
        const int N = 10000;
        GRandom rand;
        for (int i = 0; i < N; ++i) {
            GColor color = rand_color(rand);
            canvas->fillRect(fRect, color);
        }
    }
};

///////////////////////////////////////

REGISTER_GBENCH(1, return new ClearBench; );
REGISTER_GBENCH(1, return new RectsBench(false, true); );
REGISTER_GBENCH(1, return new RectsBench(true,  true); );
REGISTER_GBENCH(1, return new RectsBench(false, false); );
REGISTER_GBENCH(1, return new RectsBench(true,  false); );
REGISTER_GBENCH(1, return new SingleRectBench({2,2}, GRect::LTRB(-1000, -1000, 1002, 1002), "rect_big"); );
REGISTER_GBENCH(1, return new SingleRectBench({1000,1000}, GRect::LTRB(500, 500, 502, 502), "rect_tiny"); );
