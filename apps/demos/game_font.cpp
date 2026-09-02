

#include "../../include/GCanvas.h"
#include "../../include/GColor.h"
#include "../../include/GPoint.h"
#include "../../include/GRect.h"

const float Ascent = 5, Descent = 1;

static void draw_nib(GCanvas* canvas, GPoint tl, unsigned nib, float scale, const GColor& color) {
    for (int i = 0; i < 4; ++i) {
        if (nib & (1 << (3 - i))) {
            canvas->fillRect({tl.x, tl.y, tl.x + scale, tl.y + scale}, color);
        }
        tl.x += scale;
    }
}

static void draw_nibs(GCanvas* canvas, GPoint tl, const uint8_t nibs[], float scale, const GColor& color) {
    for (int i = 0; i < 3; ++i) {
        draw_nib(canvas, tl, nibs[i] >> 4, scale, color);
        tl.y += scale;
        draw_nib(canvas, tl, nibs[i] & 0xF, scale, color);
        tl.y += scale;
    }
}

// { char, advance, bits[] }
const uint8_t gGlyphs[] = {
    '.', 2, 0x00, 0x00, 0x80,
    '0', 4, 0x6A, 0xAA, 0xC0,
    '1', 4, 0x4C, 0x44, 0x40,   // could have advance == 3
    '2', 4, 0xC2, 0x48, 0xE0,
    '3', 4, 0xC2, 0x42, 0xC0,
    '4', 4, 0xAA, 0xE2, 0x20,
    '5', 4, 0xE8, 0xC2, 0xC0,
    '6', 4, 0x68, 0xEA, 0xE0,
    '7', 4, 0xE2, 0x48, 0x80,
    '8', 4, 0xEA, 0xEA, 0xE0,
    '9', 4, 0xEA, 0xE2, 0xC0,
    0,  // terminator
};

static const uint8_t* find_glyph(uint8_t c) {
    const uint8_t* glyphs = gGlyphs;
    while (*glyphs) {
        if (*glyphs == c) {
            return glyphs;
        }
        glyphs += 5;
    }
    return nullptr;
}

float DrawStr(GCanvas* canvas, GPoint origin, const std::string& str, float size, const GColor& color) {
    const float scale = size / (Ascent + Descent);
    float startOriginX = origin.x;

    origin.y -= Ascent * scale;
    for (size_t i = 0; i < str.size(); ++i) {
        if (auto g = find_glyph(str[i])) {
            draw_nibs(canvas, origin, &g[2], scale, color);
            origin.x += scale * g[1];
        }
    }
    return origin.x - startOriginX;
}
