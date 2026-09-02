/**
 *  Copyright 2015 Mike Reed
 */

#ifndef GTestStats_DEFINED
#define GTestStats_DEFINED

#include "../../include/GBitmap.h"
#include "../../include/GPixel.h"
#include "../auto_register.h"

extern bool gTestSuite_Verbose;
extern bool gTestSuite_CrashOnFailure;

#define EXPECT_TRUE(stats, pred)    (stats)->expectTrue(pred, __FILE__, __LINE__)
#define EXPECT_FALSE(stats, pred)   (stats)->expectFalse(pred, __FILE__, __LINE__)
#define EXPECT_EQ(stats, va, vb)    (stats)->expectEQ(va, vb, __FILE__, __LINE__)
#define EXPECT_NE(stats, va, vb)    (stats)->expectNE(va, vb, __FILE__, __LINE__)
#define EXPECT_NULL(stats, ptr)     (stats)->expectNULL(ptr, __FILE__, __LINE__)
#define EXPECT_PTR(stats, ptr)      (stats)->expectPtr(ptr, __FILE__, __LINE__)

struct GTestStats {
    GTestStats() : fTestCounter(0), fPassCounter(0) {}

    template <typename T> void expectEQ(T a, T b, const char file[], int line) {
        this->expectTrue(a == b, file, line);
    }
    
    template <typename T> void expectNE(T a, T b, const char file[], int line) {
        this->expectTrue(a != b, file, line);
    }

    void expectNULL(const void* ptr, const char file[], int line) {
        this->expectTrue(NULL == ptr, file, line);
    }
    
    void expectPtr(const void* ptr, const char file[], int line) {
        this->expectTrue(NULL != ptr, file, line);
    }

    void expectFalse(bool pred, const char file[], int line) {
        this->expectTrue(!pred, file, line);
    }
    
    void expectTrue(bool pred, const char file[], int line) {
        this->didTest(pred, file, line);
        fPassCounter += (int)pred != 0;
        fTestCounter += 1;
    }
    
    float percent() const {
        if (fTestCounter) {
            return 1.0f * fPassCounter / fTestCounter;
        } else {
            return 0;
        }
    }
    
    int fTestCounter;
    int fPassCounter;

private:
    void didTest(bool success, const char file[], int line) {
        if (gTestSuite_Verbose || !success) {
            printf("tests: %s:%d %s\n", file, line, success ? "passed" : "failed");
        }
        if (gTestSuite_CrashOnFailure) {
            assert(false);
        }
    }
};

using GTestProc = void(GTestStats*);

struct GTestRec {
    GTestProc*  fProc;
    const char* fName;
    int         fPA;
};

using TestRegistrant = GRegistrant<GTestRec>;
#define REGISTER_GTEST(proc, name, pa) \
    static TestRegistrant G_MACRO_UNIQUE_NAME(gtest_proc)({proc, name, pa})

///////////////////

template <typename S> void visit_pixels(const GBitmap& bm, S&& visitor) {
    for (int y = 0; y < bm.height(); ++y) {
        for (int x = 0; x < bm.width(); ++x) {
            visitor(x, y, bm.getAddr(x, y));
        }
    }
}

static inline void force_fill_pixels(GBitmap& bm, GPixel pixel) {
    visit_pixels(bm, [pixel](int x, int y, GPixel* p) {
        *p = pixel;
    });
}

static inline bool expect_pixels_value(const GBitmap& bm, GPixel pixel) {
    bool success = true;
    visit_pixels(bm, [&](int x, int y, GPixel* p) {
        if (*p != pixel) {
            printf("expected %x  actual %x\n", pixel, *p);
            success = false;
        }
    });
    return success;
}

#endif
