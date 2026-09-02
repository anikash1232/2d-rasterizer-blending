/**
 *  Copyright 2015 Mike Reed
 */

#include "../include/GBitmap.h"

void GBitmap::computeIsOpaque() {
    auto compute = [this]() {
        for (int y = 0; y < this->height(); ++y) {
            const GPixel *row = this->getAddr(0, y);
            for (int x = 0; x < this->width(); ++x) {
                if (GPixel_GetA(row[x]) != 0xFF) {
                    return false;
                }
            }
        }
        return true;
    };
    fIsOpaque = compute();
}

GBitmap::GBitmap(GISize sz, size_t rb, std::shared_ptr<GData> pixels) {
    assert(sz.width >= 0);
    assert(sz.height >= 0);
    if (rb == 0) {
        rb = sz.width * sizeof(GPixel);
    }

    fSize = sz;
    fRowBytes = rb;
    if (pixels) {
        fPixels = std::move(pixels);
    } else {
        fPixels = GData::Zeroed(fSize.height * rb);
    }
    this->validate();
}
