/**
 *  Copyright 2021 Mike Reed
 */

#include "../../include/GCanvas.h"
#include "../../include/GBitmap.h"
#include "../../include/GColor.h"
#include "../../include/GPoint.h"
#include "../../include/GRect.h"
#include "tests.h"

static void test_clear(GTestStats* stats) {
    const struct {
        GColor color;
        GPixel pixel;
    } rec[] = {
        { {1,0,0,1},     GPixel_PackARGB(0xFF, 0xFF, 0, 0) },
        { {0,1,0,1},     GPixel_PackARGB(0xFF, 0, 0xFF, 0) },
        { {0,0,1,1},     GPixel_PackARGB(0xFF, 0, 0, 0xFF) },
        { {1,0,0,0.25f}, GPixel_PackARGB(0x40, 0x40, 0, 0) },
        { {0,1,0,0.5f},  GPixel_PackARGB(0x80, 0, 0x80, 0) },
        { {0,0,1,0.75f}, GPixel_PackARGB(0xBF, 0, 0, 0xBF) },
        { {0,0,0,0},     GPixel_PackARGB(0, 0, 0, 0) },
        { {1,1,1,0},     GPixel_PackARGB(0, 0, 0, 0) },
    };

    for (const auto& r : rec) {
        GBitmap bm({3, 3});
        auto canvas = GCreateCanvas(bm);
        force_fill_pixels(bm, 0x12345678);  // init with garbage
        canvas->clear(r.color);
        EXPECT_TRUE(stats, expect_pixels_value(bm, r.pixel));
    }
}

static void test_rect_nodraw(GTestStats* stats) {
    const GRect rects[] = {
        {0, 0, 0, 0},
        // doesn't cross a pixel center
        {0.51f, 0, 1.49f, 3},
        {0, 0.51f, 3, 1.49f},
        // clipped out
        {-5, 0, 0, 3},
        {3, 0, 5, 3},
        {0, 3, 3, 5},
        {0 -3, 3, 0},
    };
    
    const GColor color = GColor_white;
    for (const auto& r : rects) {
        GBitmap bm({3, 3});
        auto canvas = GCreateCanvas(bm);
        canvas->fillRect(r, color);
        EXPECT_TRUE(stats, expect_pixels_value(bm, 0));
    }
}

static void edgecase_x() {
    GBitmap bm({1, 5});
    auto canvas = GCreateCanvas(bm);

    float almost = std::nexttowardf(1, -1);

    const std::pair<GPoint, GPoint> pts[] = {
        {{1, 0}, {almost, 5}},
        {{almost, 0}, {1, 5}},
        {{0, 0}, {1, 1.5f}},
        {{0, 0}, {4.0f/3, 2}},
    };
    for (auto p : pts) {
        canvas->hairLine(p.first, p.second, GColor_white);
    }
}

static void edgecase_y() {
    GBitmap bm({5, 1});
    auto canvas = GCreateCanvas(bm);

    float almost = std::nexttowardf(1, -1);
    const std::pair<GPoint, GPoint> pts[] = {
        {{0, 1}, {5, almost}},
        {{0, almost}, {5, 1}},
        {{0, 0}, {1.5f, 1}},
        {{0, 0}, {2, 4.0f/3}},
    };
    for (auto p : pts) {
        canvas->hairLine(p.first, p.second, GColor_white);
    }
}

static void test_hairlines_nodraw(GTestStats* stats) {
    GBitmap bm({5, 5});
    auto canvas = GCreateCanvas(bm);

    const GPoint pairs[] = {
        {-1, -1}, {-1, 6},
        {6, -1}, {6, 6},
        {-1, -1}, {6, -1},
        {-1, 6}, {6, 6},

        {2.1f, 2}, {2.49f, 2},
        {2, 2.1f}, {2, 2.49f},

        {5, 0}, {5, 5},
        {0, 5}, {5, 5},
    };
    for (size_t i = 0; i+1 < std::size(pairs); i += 2) {
        canvas->hairLine(pairs[i+0], pairs[i+1], GColor_white);
        EXPECT_TRUE(stats, expect_pixels_value(bm, 0));
    }

    // Make sure these don't assert/crash, as they place lines right near
    // the right and bottom of the device.
    edgecase_x();
    edgecase_y();
}

//////////////

REGISTER_GTEST(test_clear,            "clear",        1);
REGISTER_GTEST(test_hairlines_nodraw, "lines_nodraw", 1);
REGISTER_GTEST(test_rect_nodraw,      "rect_nodraw",  1);
