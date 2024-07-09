/* fwp.h -- https://github.com/takeiteasy/fwp
 
 The MIT License (MIT)

 Copyright (c) 2024 George Watson

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

#ifndef FWP_HEADER
#define FWP_HEADER
#if defined(__cplusplus)
extern "C" {
#endif

#include "pb.h"
#include "rng.h"

typedef struct fwpState fwpState;

typedef enum {
#define X(NAME, ARGS) NAME##Event,
    FWP_PB_CALLBACKS
#undef X
} pbEventType;

typedef struct {
    struct {
        int button;
        int isdown;
        struct {
            unsigned int x, y;
            float dx, dy;
        } position;
        struct {
            float dx, dy;
        } wheel;
    } mouse;
    struct {
        pbKey key;
        int isdown;
    } keyboard;
    pbMod modifier;
    struct {
        int focused, closed;
        struct {
            unsigned int width, height;
        } size;
    } window;
    pbEventType type;
} pbEvent;

typedef struct {
    int windowWidth;
    int windowHeight;
    const char *windowTitle;
    fwpState*(*init)(pbImage*);
    void(*deinit)(fwpState*);
    void(*reload)(fwpState*);
    void(*unload)(fwpState*);
    int(*event)(fwpState*, pbEvent*);
    int(*tick)(fwpState*, pbImage*, double);
} fwpScene;

#if defined(__cplusplus)
}
#endif
#endif // FWP_HEADER
