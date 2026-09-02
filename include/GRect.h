/**
 *  Copyright 2015 Mike Reed
 */

#ifndef GRect_DEFINED
#define GRect_DEFINED

#include "GMath.h"
#include "GPoint.h"

struct GIRect {
    int32_t left, top, right, bottom;

    int32_t x() const { return left; }
    int32_t y() const { return top; }
    int32_t width() const { return right - left; }
    int32_t height() const { return bottom - top; }

    static GIRect LTRB(int32_t l, int32_t t, int32_t r, int32_t b) {
        return {l, t, r, b};
    }
    
    static GIRect XYWH(int32_t x, int32_t y, int32_t w, int32_t h) {
        return {x, y, x + w, y + h};
    }
    
    static GIRect WH(int32_t w, int32_t h) {
        return {0, 0, w, h};
    }

    GIRect offset(int dx, int dy) const {
        return {left + dx, top + dy, right + dx, bottom + dy};
    }
    GIRect inset(int dx, int dy) const {
        return LTRB(left + dx, top + dy, right - dx, bottom - dy);
    }
    GIRect outset(int dx, int dy) const {
        return this->inset(-dx, -dy);
    }

    bool empty() const { return left >= right || top >= bottom; }

    operator bool() const { return !this->empty(); }
};

struct GRect {
    float left, top, right, bottom;

    float x() const { return left; }
    float y() const { return top; }
    float width() const { return right - left; }
    float height() const { return bottom - top; }

    GPoint TL() const { return {left, top}; }
    GPoint TR() const { return {right, top}; }
    GPoint BL() const { return {left, bottom}; }
    GPoint BR() const { return {right, bottom}; }

    float cx() const { return (left + right) * 0.5f; }
    float cy() const { return (top + bottom) * 0.5f; }
    GPoint center() const { return { this->cx(), this->cy() }; }

    static GRect LTRB(float l, float t, float r, float b) {
        return {l, t, r, b};
    }
    
    static GRect XYWH(float x, float y, float w, float h) {
        return {x, y, x + w, y + h};
    }
    
    static GRect WH(float w, float h) {
        return {0, 0, w, h};
    }
    
    GRect offset(float dx, float dy) const {
        return LTRB(left + dx, top + dy, right + dx, bottom + dy);
    }

    GRect offset(GVector v) const { return this->offset(v.x, v.y); }

    GRect inset(float dx, float dy) const {
        return LTRB(left + dx, top + dy, right - dx, bottom - dy);
    }
    GRect outset(float dx, float dy) const {
        return this->inset(-dx, -dy);
    }

    bool empty() const { return left >= right || top >= bottom; }

    operator bool() const { return !this->empty(); }

    GIRect round() const {
        return GIRect::LTRB(GRoundToInt(left), GRoundToInt(top),
                            GRoundToInt(right), GRoundToInt(bottom));
    }

    GIRect roundOut() const {
        return GIRect::LTRB(GFloorToInt(left), GFloorToInt(top),
                            GCeilToInt(right), GCeilToInt(bottom));
    }
};

#endif
