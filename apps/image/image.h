/**
 *  Copyright 2015 Mike Reed
 */

#ifndef G_image_DEFINED
#define G_image_DEFINED

#include "../auto_register.h"
#include "../../include/GPoint.h"

class GCanvas;

using GDrawProc = void(GCanvas*);

struct GDrawRec {
    GDrawProc*  fDraw;
    GISize      fSize;
    const char* fName;
    int         fPA;
};

using ImageRegistrant = GRegistrant<GDrawRec>;
#define REGISTER_GIMAGE(proc, w, h, name, pa) \
    static ImageRegistrant G_MACRO_UNIQUE_NAME(gimage_proc)({proc, {w, h}, name, pa})

#endif
