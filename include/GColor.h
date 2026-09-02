/**
 *  Copyright 2015 Mike Reed
 */

#ifndef GColor_DEFINED
#define GColor_DEFINED

#include "GMath.h"

/**
 *  Defines an un-premultiplied color, where alpha, red, green, blue are all independent of
 *  each other. Legal values for each component are [0.0 .. 1.0].
*/
struct GColor {
    float r, g, b, a;

    static GColor RGBA(float r, float g, float b, float a) {
        return {r, g, b, a};
    }

    static GColor RGB(float r, float g, float b) {
        return {r, g, b, 1};
    }

    bool operator==(const GColor& c) const {
        return r == c.r &&
               g == c.g &&
               b == c.b &&
               a == c.a;
    }
    bool operator!=(const GColor& c) const { return !(*this == c); }

    GColor operator-() const { return {-r, -g, -b, -a}; }
    GColor operator+(const GColor& c) const { return { r + c.r, g + c.g, b + c.b, a + c.a }; }
    GColor operator-(const GColor& c) const { return { r - c.r, g - c.g, b - c.b, a - c.a }; }
    GColor operator*(const GColor& c) const { return { r * c.r, g * c.g, b * c.b, a * c.a }; }

    friend GColor operator*(const GColor& c, float s) { return { c.r*s, c.g*s, c.b*s, c.a*s }; }
    friend GColor operator*(float s, const GColor& c) { return c*s; }

    GColor& operator+=(const GColor& c) { *this = *this + c; return *this; }
    GColor& operator-=(const GColor& c) { *this = *this - c; return *this; }
    GColor& operator*=(const GColor& c) { *this = *this * c; return *this; }

    GColor withAlpha(float newA) const {
        assert(newA >= 0 && newA <= 1);
        return {r, g, b, newA};
    }
    GColor darken(float s) const {
        return {r * s, g * s, b * s, a};
    }
    GColor lighten(float s) const {
        auto lerp = [s](float component) {
            return (1 - s) * component + s;
        };
        return {lerp(r), lerp(g), lerp(b), a};
    }
};

constexpr GColor GColor_black       = {0, 0, 0, 1};
constexpr GColor GColor_dkGray      = {0.25f, 0.25f, 0.25f, 1};
constexpr GColor GColor_gray        = {0.5f, 0.5f, 0.5f, 1};
constexpr GColor GColor_ltGray      = {0.75f, 0.75f, 0.75f, 1};
constexpr GColor GColor_white       = {1, 1, 1, 1};
constexpr GColor GColor_red         = {1, 0, 0, 1};
constexpr GColor GColor_green       = {0, 1, 0, 1};
constexpr GColor GColor_blue        = {0, 0, 1, 1};
constexpr GColor GColor_cyan        = {0, 1, 1, 1};
constexpr GColor GColor_magenta     = {1, 0, 1, 1};
constexpr GColor GColor_yellow      = {1, 1, 0, 1};
constexpr GColor GColor_transparent = {0, 0, 0, 0};

#endif
