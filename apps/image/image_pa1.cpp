/**
 *  Copyright 2015 Mike Reed
 */

#include "image.h"
#include "../../include/GCanvas.h"
#include "../../include/GBitmap.h"
#include "../../include/GColor.h"
#include "../../include/GPoint.h"
#include "../../include/GRandom.h"
#include "../../include/GRect.h"
#include <string>

static inline GColor rand_color(GRandom& rand, float a = -1) {
    auto r = rand.nextF(),
            g = rand.nextF(),
            b = rand.nextF();
    if (a < 0) {
        a = rand.nextF();
    } else {
        assert(a <= 1);
    }
    return {r, g, b, a};
}

static void dr_wheel(GCanvas* canvas, GPoint center, float radius, float alpha = -1) {
    GRandom rand;
    const int N = 250;
    for (int i = 0; i < N; ++i) {
        float theta = i * gFloatPI * 2 / N;
        auto p = GPoint{std::cos(theta), std::sin(theta)} * radius;
        canvas->hairLine(center, center + p, rand_color(rand, alpha));
    }
}

static void draw_wheel(GCanvas* canvas) {
    dr_wheel(canvas, {128, 128}, 120);
}

static void draw_wheels_clipped(GCanvas* canvas) {
    const float radius = 120;
    dr_wheel(canvas, {  0,   0}, radius, 1);
    dr_wheel(canvas, {250,   0}, radius, 1);
    dr_wheel(canvas, {  0, 256}, radius, 1);
    dr_wheel(canvas, {256, 256}, radius, 1);
}

static void draw_solid_ramp(GCanvas* canvas) {
    const float c = 1.0 / 512;
    const float d = 1.0 / 256;

    const struct {
        GColor  fC0, fDC;
    } rec[] = {
        { {  c,   c,   c, 1}, { d,  d,  d, 0} },   // grey
        { {1-c,   0,   0, 1}, {-d,  0,  0, 0} },   // red
        { {  0,   c,   c, 1}, { 0,  d,  d, 0} },   // cyan
        { {  0, 1-c,   0, 1}, { 0, -d,  0, 0} },   // green
        { {  c,   0,   c, 1}, { d,  0,  d, 0} },   // magenta
        { {  0,   0, 1-c, 1}, { 0,  0, -d, 0} },   // blue
        { {  c,   c,   0, 1}, { d,  d,  0, 0} },   // yellow
    };

    
    for (size_t y = 0; y < std::size(rec); ++y) {
        GColor color = rec[y].fC0;
        GColor delta = rec[y].fDC;
        for (int x = 0; x < 256; x++) {
            const GRect rect = GRect::XYWH(x, y * 28, 1, 28);
            canvas->fillRect(rect, color);
            color += delta;
        }
    }
}

static void draw_blend_ramp(GCanvas* canvas, const GColor& bg) {
    GRect rect = GRect::XYWH(-25, -25, 70, 70);

    int delta = 8;
    for (int i = 0; i < 200; i += delta) {
        float r = i / 200.0f;
        float g = std::abs(std::cos(i/40.0f));
        float b = std::abs(std::sin(i/50.0f));
        GColor color = {r, g, b, 0.3f};
        canvas->fillRect(rect, color);
        rect = rect.offset(delta, delta);
    }
}

struct Graph {
    int fSteps;
    float fMin, fMax;
    GPoint fScale = { 1, 1 };
    GPoint fOffset = { 0, 0 };
};

static void graph_rects(GCanvas* canvas, const Graph& g,
                        float (*func)(float x), GColor (*color)(float t)) {
    const float dx = (g.fMax - g.fMin) / (g.fSteps - 1);
    const float halfwidth = dx * g.fScale.x * 0.5f;
    const float y0 = g.fOffset.y;
    float x = g.fMin;
    for (int i = 0; i < g.fSteps; ++i) {
        float y = func(x);
        float sx = x * g.fScale.x + g.fOffset.x;
        float sy = -y * g.fScale.y + g.fOffset.y;
        GRect r = GRect::LTRB(sx - halfwidth, sy, sx + halfwidth, y0);
        if (r.height() < 0) {
            std::swap(r.top, r.bottom);
        }
        r.left += 0.5f;
        r.right -= 0.5f;
        canvas->fillRect(r, color((x - g.fMin) / (g.fMax - g.fMin)));
        x += dx;
    }
}

static void draw_graphs(GCanvas* canvas) {
    Graph g;
    g.fSteps = 30;
    g.fMin = -gFloatPI;
    g.fMax = gFloatPI;
    g.fScale = { 40, 60 };
    g.fOffset = { 128, 128 };
    graph_rects(canvas, g, [](float x) {
                    return sinf(x);
                },
                [](float t) -> GColor {
                    return {t, 0, 1 - t, 1};
                });

    g.fSteps = 70;
    g.fMin = -5*gFloatPI;
    g.fMax =  5*gFloatPI;
    g.fScale = { 10, 40 };
    g.fOffset = { 128, 40 };
    graph_rects(canvas, g, [](float x) {
                    return sinf(x) / x;
                },
                [](float t) -> GColor { return {0, 0.5, 0, 1}; }
                );

    g.fSteps = 20;
    g.fMin = 0;
    g.fMax = 1;
    g.fScale = { 100, 100 };
    g.fOffset = { 128, 250 };
    graph_rects(canvas, g,
                [](float x) { return sqrtf(x); },
                [](float t) -> GColor { float c = (1 - t) * 0.75f; return {c, c, c, 1}; }
                );
}

static void draw_blend_black(GCanvas* canvas) {
    draw_blend_ramp(canvas, {0, 0, 0, 1});
}

//////////////

REGISTER_GIMAGE(draw_wheel,           256, 256 , "wheel",          1);
REGISTER_GIMAGE(draw_wheels_clipped,  256, 256,  "wheels_clipped", 1);
REGISTER_GIMAGE(draw_solid_ramp,      256, 7*28, "solid_ramp",     1);
REGISTER_GIMAGE(draw_graphs,          256, 256,  "rect_graphs",    1);
REGISTER_GIMAGE(draw_blend_black,     200, 200,  "blend_black",    1);
