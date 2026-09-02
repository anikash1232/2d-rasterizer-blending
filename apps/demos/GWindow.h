/**
 *  Copyright 2015 Mike Reed
 */

#ifndef GWindow_DEFINED
#define GWindow_DEFINED

#include <SDL2/SDL.h>
#include <functional>

#include "../../include/GBitmap.h"
#include "../../include/GPoint.h"

class GCanvas;
class GClick;
class GIRect;

enum GFastKeys {
    left_arrow =  1 << 0,
    right_arrow = 1 << 1,
    up_arrow =    1 << 2,
    down_arrow =  1 << 3,
    return_key =  1 << 4,
    space_key =   1 << 5,

    ctrl_key  = 1 << 6,
};

class GWindow {
public:
    int run();

    void requestDraw();

protected:
    GWindow(GISize);
    virtual ~GWindow();

    GISize size() const { return fSize; }

    uint32_t pollFastKeys() const;

    virtual void onUpdate(GCanvas*);
    virtual void onDraw(GCanvas*) {}
    virtual void onResize(GISize) {}
    virtual bool onKeyPress(uint32_t) { return false; }
    virtual void onKeyUp(uint32_t) {}
    virtual void onFastKeys(uint32_t mask) {}
    virtual GClick* onFindClickHandler(GPoint) { return NULL; }

    void setTitle(const char title[]);

private:
    GClick*     fClick;
    
    GISize fSize;
    bool fNeedDraw;

    SDL_Window*   fWindow;
    SDL_Renderer* fRenderer;
    SDL_Texture*  fTexture;
    const uint8_t* fKeyboardState;

    uint32_t fInvalEventType;

    bool handleEvent(const SDL_Event&);
    void pushEvent(int code) const;
    void drawIntoTexture();
};

class GClick {
public:
    GClick(GPoint, std::function<void(GClick*)>);
    
    enum State {
        kDown_State,
        kMove_State,
        kUp_State
    };
    
    State state() const { return fState; }
    GPoint curr() const { return fCurr; }
    GPoint prev() const { return fPrev; }
    GPoint orig() const { return fOrig; }

    void callback() { fFunc(this); }

private:
    GPoint  fCurr, fPrev, fOrig;
    State   fState;
    std::function<void(GClick*)> fFunc;

    friend class GWindow;
};

#endif
