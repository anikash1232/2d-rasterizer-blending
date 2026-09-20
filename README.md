# 2D Rasterizer — Compositing

A software rasterizer in C++ implementing the complete Porter-Duff compositing model. No
GPU, no graphics library — every pixel is computed and written by hand.

## What it does

Renders shapes into a bitmap, combining each new shape with what's already there according
to a selected blend mode. All twelve Porter-Duff operators are supported, which covers the
full space of ways two premultiplied images can be composited:

```
kClear   kSrc      kDst      kSrcOver   kDstOver   kSrcIn
kDstIn   kSrcOut   kDstOut   kSrcATop   kDstATop   kXor
```

## How it works

**One formula, twelve modes.** Every Porter-Duff operator reduces to the same shape:
`result = ka·src + kb·dst`, differing only in how `ka` and `kb` derive from the two alpha
values. Rather than writing twelve blend functions, `blend_pixel` selects the coefficient
pair and runs one expression:

```cpp
case GBlendMode::kSrcOver: ka = 255;      kb = 255 - sa; break;
case GBlendMode::kSrcIn:   ka = da;       kb = 0;        break;
case GBlendMode::kXor:     ka = 255 - da; kb = 255 - sa; break;
```

**Paying for the switch once, not per pixel.** A naive implementation branches on the mode
for every pixel in every span. Here `blend_span` is templated on the mode:

```cpp
template <GBlendMode M>
static void blend_span(GPixel* p, GPixel* end, GPixel src) {
    for (; p < end; ++p) *p = blend_pixel(M, src, *p);
}
```

Because `M` is a compile-time constant, the compiler folds the switch away entirely and
emits a specialised inner loop per mode — the arithmetic for exactly one operator, with no
branch in the hot path.

**Integer math throughout.** Premultiplied 8-bit channels are divided by 255 via `div255`,
which uses the standard shift-and-add approximation instead of an integer division in the
innermost loop.

## Building

```bash
make
./tests
```

Requires a C++17 compiler.
