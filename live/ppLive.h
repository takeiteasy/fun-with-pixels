/* ppLive.h -- https://github.com/takeiteasy/pp
 
 The MIT License (MIT)

 Copyright (c) 2022 George Watson

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software,
 and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#ifndef ppLive_h
#define ppLive_h
#if defined(__cplusplus)
extern "C" {
#endif

#include "pp.h"

typedef union ppColor {
    struct {
        unsigned char a;
        unsigned char r;
        unsigned char g;
        unsigned char b;
    };
    int value;
} ppColor;

typedef struct ppSurface {
    int *buf, w, h;
} ppSurface;

EXPORT ppSurface* ppNewSurface(int w, int h);
EXPORT void ppFreeSurface(ppSurface *surface);
EXPORT void ppClearSurface(ppSurface *surface, ppColor color);
EXPORT void ppSurfacePSet(ppSurface *surface, int x, int y, ppColor color);
EXPORT ppColor ppSurfacePGet(ppSurface *surface, int x, int y);

typedef struct ppState ppState;

typedef enum {
#define X(NAME, ARGS) pp##NAME##Event,
    PP_CALLBACKS
#undef X
} ppEventType;

typedef struct {
    struct {
        int button;
        int isdown;
        struct {
            int x, y;
            float dx, dy;
        } Position;
        struct {
            float dx, dy;
        } Scroll;
    } Mouse;
    struct {
        ppKey key;
        int isdown;
    } Keyboard;
    ppMod modifier;
    struct {
        int focused, closed;
        struct {
            int width, height;
        } Size;
    } Window;
    ppEventType type;
} ppEvent;

typedef struct {
    ppState*(*init)(void);
    void(*deinit)(ppState*);
    void(*reload)(ppState*);
    void(*unload)(ppState*);
    int(*event)(ppState*, ppEvent*);
    int(*tick)(ppState*, ppSurface*, double);
} ppApp;

#ifndef _MSC_VER
extern const ppApp pp;
#endif

#if defined(__cplusplus)
}
#endif
#endif // ppLive_h
