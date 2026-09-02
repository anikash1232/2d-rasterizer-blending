/**
 *  Copyright 2015 Mike Reed
 */

#ifndef GBitmap_DEFINED
#define GBitmap_DEFINED

#include "GPixel.h"
#include "GPoint.h"
#include "GData.h"

#include <optional>

class GBitmap {
public:
    GBitmap()
        :  fPixels(nullptr)
        , fSize({0, 0})
        , fRowBytes(0)
        , fIsOpaque(false)
    {}

    GBitmap(GISize sz, size_t rb = 0, std::shared_ptr<GData> = nullptr);

    int width() const { return fSize.width; }
    int height() const { return fSize.height; }
    size_t rowBytes() const { return fRowBytes; }
    GPixel* pixels() const { return fPixels ? (GPixel*)fPixels->data() : nullptr; }
    bool isOpaque() const { return fIsOpaque; }

    GPixel* getAddr(int x, int y) const {
        assert(x >= 0 && x < this->width());
        assert(y >= 0 && y < this->height());
        return this->pixels() + x + (y * this->rowBytes() >> 2);
    }

    /**
     * Inspects all pixels' alpha, and sets the bitmap's isOpaque attribute to true iff
     * all of the pixels are opaque.
     *
     * Note: this always (re)computes the value, regardless of the current setting.
     */
    void computeIsOpaque();

    /**
     *  Attempt to read the png image stored in the named file.
     *
     *  On success, allocate the memory for the pixels using malloc() and set bitmap to the result,
     *  returning true. The caller must call free(bitmap->fPixels) when they are finished.
     *
     *  This automatically computes the opaqueness of the bitmap.
     *
     *  On failure, return false and bitmap is reset to empty.
     */
    static std::optional<GBitmap> ReadFromFile(const char path[]);

    /*
     *  Attempt to write the bitmap as a PNG into a new file (the file will be created/overwritten).
     *  Return true on success.
     */
    bool writeToFile(const char path[]) const;

    std::shared_ptr<GData> asPNGData() const;

private:
    std::shared_ptr<GData> fPixels;
    GISize  fSize;
    size_t  fRowBytes;
    bool    fIsOpaque = false;  // hint that all pixels have 0xFF for alpha

    void validate() const {
        assert(fSize.width >= 0);
        assert(fSize.height >= 0);
        assert((unsigned)fSize.width <= fRowBytes >> 2);
    }
};

#endif
