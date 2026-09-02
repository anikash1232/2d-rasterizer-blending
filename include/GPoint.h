/**
 *  Copyright 2015 Mike Reed
 */

#ifndef GPoint_DEFINED
#define GPoint_DEFINED

#include "GMath.h"

struct GPoint {
    float x, y;

    float length() const { return sqrtf(x*x + y*y); }

    bool operator==(GPoint p) const {
        return x == p.x && y == p.y;
    }
    bool operator!=(GPoint p) const { return !(*this == p); }

    GPoint operator+(GPoint v) const {
        return { x + v.x, y + v.y };
    }
    GPoint operator-(GPoint v) const {
        return { x - v.x, y - v.y };
    }

    friend GPoint operator*(GPoint v, float s) {
        return { v.x * s, v.y * s };
    }
    friend GPoint operator*(float s, GPoint v) {
        return { v.x * s, v.y * s };
    }
    friend GPoint operator/(GPoint v, float s) {
        return { v.x / s, v.y / s };
    }

    GPoint& operator+=(GPoint o) {
        x += o.x;
        y += o.y;
        return *this;
    }
    GPoint& operator-=(GPoint o) {
        x -= o.x;
        y -= o.y;
        return *this;
    }
    GPoint& operator*=(float s) {
        x *= s;
        y *= s;
        return *this;
    }
    GPoint& operator/=(float s) {
        x /= s;
        y /= s;
        return *this;
    }
};
using GVector = GPoint;

template <typename T> struct GTSize {
    T width, height;
};
typedef GTSize<int> GISize;
typedef GTSize<float> GSize;

#endif
