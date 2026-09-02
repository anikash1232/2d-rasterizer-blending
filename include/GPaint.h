/*
 *  Copyright 2016 Mike Reed
 */

#ifndef GPaint_DEFINED
#define GPaint_DEFINED

#include "GBlendMode.h"
#include "GColor.h"

class GPaint {
public:
    GPaint() {}
    explicit GPaint(const GColor& c) : fColor(c) {}

    GColor color() const { return fColor; }
    void setColor(GColor c) { fColor = c; }

    float alpha() const { return fColor.a; }
    void setAlpha(float alpha) { fColor.a = alpha; }

    GBlendMode blendMode() const { return fMode; }
    void setBlendMode(GBlendMode m) { fMode = m; }

    // Only used for drawLine()
    // If width < 0 draw a hairline, otherwise use width to construct the stroked line.
    float lineWidth() const { return fLineWidth; }
    bool isHairline() const { return fLineWidth < 0; }
    void setLineWidth(float w) { fLineWidth = w; }
    void setHairline() { this->setLineWidth(-1); }

private:
    GColor      fColor = GColor_black;
    GBlendMode  fMode = GBlendMode::kSrcOver;
    float       fLineWidth = 1;
};

#endif
