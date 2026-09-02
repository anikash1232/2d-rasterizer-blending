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

#include <array>
#include <string>

static void make_regular_poly(GPoint pts[], int count, float cx, float cy, float radius) {
    float angle = 0;
    const float deltaAngle = gFloatPI * 2 / count;

    for (int i = 0; i < count; ++i) {
        pts[i] = {cx + std::cos(angle) * radius, cy + std::sin(angle) * radius};
        angle += deltaAngle;
    }
}

static void dr_poly(GCanvas* canvas, float dx, float dy) {
    GPoint storage[12];
    for (int count = 12; count >= 3; --count) {
        make_regular_poly(storage, count, 256, 256, count * 10 + 120);
        for (int i = 0; i < count; ++i) {
            storage[i].x += dx;
            storage[i].y += dy;
        }
        GColor c = GColor::RGBA(std::abs(sinf(count*7)),
                                std::abs(sinf(count*11)),
                                std::abs(sinf(count*17)),
                                0.8f);
        canvas->drawConvexPolygon(storage, count, GPaint(c));
    }
}

static void draw_poly(GCanvas* canvas) {
    dr_poly(canvas, 0, 0);
}

static void draw_poly_center(GCanvas* canvas) {
    dr_poly(canvas, -128, -128);
}

////////////////////////////////////////////////////////////////////////////////////

static void outer_frame(GCanvas* canvas, const GRect& rect) {
    GPaint paint;
    paint.setLineWidth(1);
    auto hline = [&](GPoint p, float w) {
        canvas->drawLine(p, {p.x + w, p.y}, paint);
    };
    auto vline = [&](GPoint p, float h) {
        canvas->drawLine(p, {p.x, p.y + h}, paint);
    };
    auto r = rect.outset(1.5f, 1.5f);
    hline({r.left - 0.5f, r.top}, r.width() + 1);
    hline({r.left - 0.5f, r.bottom}, r.width() + 1);
    vline({r.left, r.top - 0.5f}, r.height() + 1);
    vline({r.right, r.top - 0.5f}, r.height() + 1);
}

// so we test the polygon code
static std::array<GPoint, 4> rect_pts(const GRect& r) {
    return { r.TL(), r.TR(), r.BR(), r.BL() };
}

using FillRectProc = void(GCanvas*, const GRect&, const GPaint&);

static void draw_mode_sample(GCanvas* canvas, const GRect& bounds, GBlendMode mode, FillRectProc proc) {
    const float dx = bounds.width() / 3;
    const float dy = bounds.height() / 3;

    outer_frame(canvas, bounds);

    GPaint paint;
    // dst is red
    paint.setBlendMode(GBlendMode::kSrc);
    GRect r = bounds;
    r.bottom = r.top + dy;
    const GColor colors[] = {
        GColor_transparent,
        GColor_red.withAlpha(0.5),
        GColor_red,
    };
    for (auto c : colors) {
        paint.setColor(c);
        canvas->drawConvexPolygon(rect_pts(r).data(), 4, paint);
        r = r.offset(0, dy);
    }

    // src is blue
    paint.setBlendMode(mode);
    r = bounds;
    r.right = r.left + dx;
    const GColor colors2[] = {
        GColor_transparent,
        GColor_blue.withAlpha(0.5),
        GColor_blue,
    };
    for (auto c : colors2) {
        paint.setColor(c);
        proc(canvas, r, paint);
        r = r.offset(dx, 0);
    }
}

static void do_blendmodes(GCanvas* canvas, FillRectProc proc) {
    canvas->clear({1,1,1,1});

    const float W = 100;
    const float H = 100;
    const float margin = 10;
    float x = margin;
    float y = margin;
    for (int i = 0; i < 12; ++i) {
        GBlendMode mode = static_cast<GBlendMode>(i);
        draw_mode_sample(canvas, GRect::XYWH(x, y, W, H), mode, proc);
        if (i % 4 == 3) {
            y += H + margin;
            x = margin;
        } else {
            x += W + margin;
        }
    }
}

static void rect_blendmodes(GCanvas* canvas) {
    do_blendmodes(canvas, [](GCanvas* canvas, const GRect& r, const GPaint& paint) {
        canvas->drawRect(r, paint);
    });
}

static void line_blendmodes(GCanvas* canvas) {
    do_blendmodes(canvas, [](GCanvas* canvas, const GRect& r, const GPaint& paint) {
        auto ir = r.round();
        for (float y = (float)ir.top + 0.5f; y < (float)r.bottom; y += 1) {
            canvas->drawLine({r.left, y}, {r.right, y}, paint);
        }
    });
}

static void fat_lines(GCanvas* canvas) {
    GPaint paint;
    paint.setLineWidth(10);
    const GPoint c = {128, 128};
    for (int angle = 0; angle < 360; angle += 10) {
        const float rad = angle * gFloatPI / 180;
        paint.setLineWidth(14 + std::cos(rad*3) * 8);
        paint.setColor({
            std::abs(std::cos(rad*5.3f)),
            std::abs(std::cos(rad*3.7f)),
            std::abs(std::sin(rad*4.1f)),
            0.75
        });
        const GPoint pt {std::cos(rad), std::sin(rad)};
        canvas->drawLine(c + pt * 20, c + pt * 120, paint);
    }
}

//////////////

REGISTER_GIMAGE(draw_poly,        512, 512,   "poly",               2);
REGISTER_GIMAGE(draw_poly_center, 256, 256,   "poly_center",        2);
REGISTER_GIMAGE(rect_blendmodes,  450, 340,   "rect_blendmodes",    2);
REGISTER_GIMAGE(line_blendmodes,  450, 340,   "line_blendmodes",    2);
REGISTER_GIMAGE(fat_lines,        256, 256,   "fat_lines",          2);
