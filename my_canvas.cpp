#include "include/GCanvas.h"
#include "include/GBitmap.h"
#include "include/GColor.h"
#include "include/GMath.h"
#include "include/GPaint.h"
#include "include/GPoint.h"
#include "include/GRect.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
// color / blend helpers

static inline unsigned to_byte(float value) {
    return static_cast<unsigned>(GRoundToInt(std::max(0.0f, std::min(1.0f, value)) * 255));
}

// Convert an un-premultiplied GColor into a premultiplied GPixel.
static GPixel color_to_pixel(const GColor& color) {
    const float alpha = std::max(0.0f, std::min(1.0f, color.a));
    const unsigned a = to_byte(alpha);
    return GPixel_PackARGB(a, to_byte(color.r * alpha), to_byte(color.g * alpha),
                           to_byte(color.b * alpha));
}

// Rounded division by 255 for values in [0, 255*255].
static inline unsigned div255(unsigned x) {
    return (x + 127) / 255;
}

// Fast SRC_OVER: src + (1 - Sa)*dst, all premultiplied.
static inline GPixel src_over(GPixel s, GPixel d) {
    const unsigned ia = 255 - GPixel_GetA(s);
    return GPixel_PackARGB(
        GPixel_GetA(s) + div255(GPixel_GetA(d) * ia),
        GPixel_GetR(s) + div255(GPixel_GetR(d) * ia),
        GPixel_GetG(s) + div255(GPixel_GetG(d) * ia),
        GPixel_GetB(s) + div255(GPixel_GetB(d) * ia));
}

// General Porter-Duff blend for premultiplied pixels. Every mode reduces to
// result = ka*S + kb*D, where ka, kb are coefficients in [0, 255].
static inline GPixel blend_pixel(GBlendMode mode, GPixel s, GPixel d) {
    const int sa = GPixel_GetA(s);
    const int da = GPixel_GetA(d);
    int ka, kb;
    switch (mode) {
        case GBlendMode::kClear:   return 0;
        case GBlendMode::kSrc:     return s;
        case GBlendMode::kDst:     return d;
        case GBlendMode::kSrcOver: ka = 255;      kb = 255 - sa; break;
        case GBlendMode::kDstOver: ka = 255 - da; kb = 255;      break;
        case GBlendMode::kSrcIn:   ka = da;       kb = 0;        break;
        case GBlendMode::kDstIn:   ka = 0;        kb = sa;       break;
        case GBlendMode::kSrcOut:  ka = 255 - da; kb = 0;        break;
        case GBlendMode::kDstOut:  ka = 0;        kb = 255 - sa; break;
        case GBlendMode::kSrcATop: ka = da;       kb = 255 - sa; break;
        case GBlendMode::kDstATop: ka = 255 - da; kb = sa;       break;
        case GBlendMode::kXor:     ka = 255 - da; kb = 255 - sa; break;
        default:                   ka = 255;      kb = 255 - sa; break;
    }
    auto chan = [ka, kb](int sc, int dc) {
        return div255(static_cast<unsigned>(ka * sc + kb * dc));
    };
    return GPixel_PackARGB(chan(sa, da), chan(GPixel_GetR(s), GPixel_GetR(d)),
                           chan(GPixel_GetG(s), GPixel_GetG(d)),
                           chan(GPixel_GetB(s), GPixel_GetB(d)));
}

// Mode is a compile-time constant here, so blend_pixel's switch folds away and
// the coefficient selection leaves the inner loop.
template <GBlendMode M>
static void blend_span(GPixel* p, GPixel* end, GPixel src) {
    for (; p < end; ++p) {
        *p = blend_pixel(M, src, *p);
    }
}

///////////////////////////////////////////////////////////////////////////////

class BitmapCanvas final : public GCanvas {
public:
    explicit BitmapCanvas(const GBitmap& bitmap) : fBitmap(bitmap) {}

    void clear(const GColor& color) override {
        const GPixel pixel = color_to_pixel(color);
        for (int y = 0; y < fBitmap.height(); ++y) {
            GPixel* row = fBitmap.getAddr(0, y);
            std::fill(row, row + fBitmap.width(), pixel);
        }
    }

    void drawRect(const GRect& rect, const GPaint& paint) override {
        const int left = std::max(0, GRoundToInt(rect.left));
        const int top = std::max(0, GRoundToInt(rect.top));
        const int right = std::min(fBitmap.width(), GRoundToInt(rect.right));
        const int bottom = std::min(fBitmap.height(), GRoundToInt(rect.bottom));
        if (left >= right || top >= bottom) return;

        const GPixel src = color_to_pixel(paint.color());
        const GBlendMode mode = paint.blendMode();
        for (int y = top; y < bottom; ++y) {
            blendRow(y, left, right, src, mode);
        }
    }

    void drawConvexPolygon(const GPoint pts[], int count, const GPaint& paint) override {
        fillPolygon(pts, count, color_to_pixel(paint.color()), paint.blendMode());
    }

    void drawLine(GPoint p0, GPoint p1, const GPaint& paint) override {
        if (paint.isHairline()) {
            hairline(p0, p1, paint);
            return;
        }
        const GVector v = p1 - p0;
        const float len = v.length();
        if (len <= 0) return;

        const GVector u = v * (1.0f / len);
        const GVector n = {-u.y, u.x};
        const float r = paint.lineWidth() * 0.5f;
        const GPoint quad[4] = {
            p0 + n * r, p1 + n * r, p1 - n * r, p0 - n * r,
        };
        fillPolygon(quad, 4, color_to_pixel(paint.color()), paint.blendMode());
    }

private:
    struct Edge {
        float x;     // x at the top (smallest y) of the edge
        float top;   // smallest y
        float bot;   // largest y
        float slope; // dx / dy
        float curX;  // running x, stepped by slope once per scanline
        bool  started;
    };

    // Blend a span of pixels [xL, xR) on row y with a constant source pixel.
    void blendRow(int y, int xL, int xR, GPixel src, GBlendMode mode) {
        if (xL >= xR) return;
        GPixel* p = fBitmap.getAddr(xL, y);
        GPixel* end = p + (xR - xL);

        // A fully transparent source (S == 0) collapses every mode into either
        // "leave the row alone" or "write zero".
        const int srcA = GPixel_GetA(src);
        if (srcA == 0) {
            switch (mode) {
                case GBlendMode::kDst:     case GBlendMode::kSrcOver:
                case GBlendMode::kDstOver: case GBlendMode::kDstOut:
                case GBlendMode::kSrcATop: case GBlendMode::kXor:
                    return;                       // result is D
                default:
                    std::fill(p, end, 0u);        // result is 0
                    return;
            }
        }
        // A fully opaque source collapses a few more.
        if (srcA == 255) {
            switch (mode) {
                case GBlendMode::kSrcOver: std::fill(p, end, src); return;  // S
                case GBlendMode::kDstIn:   return;                          // D
                case GBlendMode::kDstOut:  std::fill(p, end, 0u);  return;  // 0
                default: break;
            }
        }

        switch (mode) {
            case GBlendMode::kClear:
                std::fill(p, end, 0u);
                return;
            case GBlendMode::kSrc:
                std::fill(p, end, src);
                return;
            case GBlendMode::kDst:
                return;
            case GBlendMode::kSrcOver: {
                const int sa = GPixel_GetA(src);
                if (sa == 255) { std::fill(p, end, src); return; }
                if (sa == 0) return;
                for (; p < end; ++p) *p = src_over(src, *p);
                return;
            }
            case GBlendMode::kDstOver: blend_span<GBlendMode::kDstOver>(p, end, src); return;
            case GBlendMode::kSrcIn:   blend_span<GBlendMode::kSrcIn>(p, end, src);   return;
            case GBlendMode::kDstIn:   blend_span<GBlendMode::kDstIn>(p, end, src);   return;
            case GBlendMode::kSrcOut:  blend_span<GBlendMode::kSrcOut>(p, end, src);  return;
            case GBlendMode::kDstOut:  blend_span<GBlendMode::kDstOut>(p, end, src);  return;
            case GBlendMode::kSrcATop: blend_span<GBlendMode::kSrcATop>(p, end, src); return;
            case GBlendMode::kDstATop: blend_span<GBlendMode::kDstATop>(p, end, src); return;
            case GBlendMode::kXor:     blend_span<GBlendMode::kXor>(p, end, src);     return;
        }
    }

    // Scanline fill of a convex polygon, using pixel-center containment.
    void fillPolygon(const GPoint pts[], int count, GPixel src, GBlendMode mode) {
        if (count < 3) return;

        fEdges.clear();
        float minY = pts[0].y, maxY = pts[0].y;
        for (int i = 0; i < count; ++i) {
            GPoint a = pts[i];
            GPoint b = pts[(i + 1) % count];
            minY = std::min(minY, a.y);
            maxY = std::max(maxY, a.y);
            if (a.y == b.y) continue; // horizontal edges contribute no scanlines
            if (a.y > b.y) std::swap(a, b);
            fEdges.push_back({ a.x, a.y, b.y, (b.x - a.x) / (b.y - a.y), 0, false });
        }
        if (fEdges.empty()) return;

        int yTop = std::max(0, GRoundToInt(minY));
        int yBot = std::min(fBitmap.height(), GRoundToInt(maxY));

        for (int y = yTop; y < yBot; ++y) {
            const float yc = y + 0.5f;
            float lo = 0, hi = 0;
            bool found = false;
            for (Edge& ed : fEdges) {
                if (yc < ed.top || yc >= ed.bot) continue;
                if (!ed.started) {
                    // keep each step separately rounded to float (no FMA contraction),
                    // so the boundary cases land where the reference puts them
                    const float dy = yc - ed.top;
                    const float dx = dy * ed.slope;
                    ed.curX = ed.x + dx;
                    ed.started = true;
                }
                const float x = ed.curX;
                ed.curX += ed.slope;   // step once per scanline, matching the reference
                if (!found) {
                    lo = hi = x;
                    found = true;
                } else {
                    lo = std::min(lo, x);
                    hi = std::max(hi, x);
                }
            }
            if (!found) continue;
            const int xL = std::max(0, GRoundToInt(lo));
            const int xR = std::min(fBitmap.width(), GRoundToInt(hi));
            blendRow(y, xL, xR, src, mode);
        }
    }

    // Hairline: one pixel per step along the major axis, sampled at the pixel
    // center and stepped incrementally. Not antialiased.
    void hairline(GPoint p0, GPoint p1, const GPaint& paint) {
        const int W = fBitmap.width(), H = fBitmap.height();

        // Trivial reject: the segment lies entirely outside the canvas. The top/left
        // cases matter: a segment whose y range is (-eps, 0] must draw nothing, but
        // GFloorToInt(0) == 0 would otherwise light up row 0.
        if (std::max(p0.y, p1.y) <= 0 || std::min(p0.y, p1.y) >= H) return;
        if (std::max(p0.x, p1.x) <= 0 || std::min(p0.x, p1.x) >= W) return;
        if (p0.x == p1.x && p0.y == p1.y) return;

        const GPixel src = color_to_pixel(paint.color());
        const GBlendMode mode = paint.blendMode();

        if (std::fabs(p1.x - p0.x) > std::fabs(p1.y - p0.y)) {
            if (p0.x > p1.x) std::swap(p0, p1);
            const float m = (p1.y - p0.y) / (p1.x - p0.x);
            const float mx0 = m * p0.x;              // kept separate so the result is
            const float b = p0.y - mx0;              // identical with or without FMA
            const int x0 = GRoundToInt(p0.x), x1 = std::min(GRoundToInt(p1.x), W);
            const float my = m * (x0 + 0.5f);
            float y = my + b;
            int x = x0;
            for (; x < 0 && x < x1; ++x, y += m) {}  // step the accumulator, nothing to draw
            for (; x < x1; ++x, y += m) {
                plot(x, GFloorToInt(y), src, mode);
            }
        } else {
            if (p0.y > p1.y) std::swap(p0, p1);
            const float m = (p1.x - p0.x) / (p1.y - p0.y);
            const float my0 = m * p0.y;              // kept separate so the result is
            const float b = p0.x - my0;              // identical with or without FMA
            const int y0 = GRoundToInt(p0.y), y1 = std::min(GRoundToInt(p1.y), H);
            const float mx = m * (y0 + 0.5f);
            float x = mx + b;
            int y = y0;
            for (; y < 0 && y < y1; ++y, x += m) {}  // step the accumulator, nothing to draw
            for (; y < y1; ++y, x += m) {
                plot(GFloorToInt(x), y, src, mode);
            }
        }
    }

    void plot(int x, int y, GPixel src, GBlendMode mode) {
        if ((unsigned)x >= (unsigned)fBitmap.width() ||
            (unsigned)y >= (unsigned)fBitmap.height()) return;
        GPixel* pixel = fBitmap.getAddr(x, y);
        if (mode == GBlendMode::kSrcOver) {
            // opaque source under SRC_OVER is just a copy
            *pixel = (GPixel_GetA(src) == 255) ? src : src_over(src, *pixel);
        } else {
            *pixel = blend_pixel(mode, src, *pixel);
        }
    }

    GBitmap fBitmap;
    std::vector<Edge> fEdges;   // reused across calls so scanning doesn't reallocate
};

///////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& bitmap) {
    return std::make_unique<BitmapCanvas>(bitmap);
}

std::string GDrawSomething(GCanvas* canvas, GISize dim) {
    canvas->clear({0.04f, 0.06f, 0.10f, 1});

    const GPoint center = {dim.width * 0.5f, dim.height * 0.5f};
    const float radius = std::min(dim.width, dim.height) * 0.42f;

    // A ring of overlapping translucent triangles fanning out from the center.
    const int blades = 12;
    for (int i = 0; i < blades; ++i) {
        const float a0 = (i * 2 * gFloatPI) / blades;
        const float a1 = ((i + 0.65f) * 2 * gFloatPI) / blades;
        const GPoint tri[3] = {
            center,
            {center.x + std::cos(a0) * radius, center.y + std::sin(a0) * radius},
            {center.x + std::cos(a1) * radius, center.y + std::sin(a1) * radius},
        };
        GColor c = GColor::RGBA(std::abs(std::sin(a0 * 1.7f)),
                                std::abs(std::sin(a0 * 2.3f + 1)),
                                std::abs(std::cos(a0 * 1.1f)), 0.55f);
        GPaint paint(c);
        paint.setBlendMode(GBlendMode::kSrcOver);
        canvas->drawConvexPolygon(tri, 3, paint);
    }

    // Thick spokes over the top, varying width and color.
    for (int i = 0; i < blades; ++i) {
        const float a = (i * 2 * gFloatPI) / blades + 0.26f;
        const GPoint tip = {center.x + std::cos(a) * radius * 1.05f,
                            center.y + std::sin(a) * radius * 1.05f};
        GPaint paint(GColor::RGBA(0.95f, 0.85f, 0.30f, 0.8f));
        paint.setLineWidth(3 + (i % 3) * 3.0f);
        canvas->drawLine(center, tip, paint);
    }

    return "Pinwheel";
}
