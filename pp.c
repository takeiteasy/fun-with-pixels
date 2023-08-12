/* ppCommon.c -- https://github.com/takeiteasy/pp
 
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

#include "pp.h"

static struct {
#define X(NAME, ARGS) void(*NAME##Callback)ARGS;
    PP_CALLBACKS
#undef X
    struct {
        int p1, p2;
        unsigned int buffer[17];
    } random;
    void *userdata;
    bool initialized;
    bool running;
    Bitmap *pbo;
} ppInternal = {0};

static bool ppBeginNative(int w, int h, const char *title, ppFlags flags);

bool ppBegin(int w, int h, const char *title, ppFlags flags) {
    if (ppInternal.running)
        return false;
    ppInternal.running = ppBeginNative(w, h, title, flags);
    return ppInternal.running;
}

#define X(NAME, ARGS) \
    void(*NAME##Callback)ARGS,
void ppCallbacks(PP_CALLBACKS void* userdata) {
#undef X
#define X(NAME, ARGS) \
    ppInternal.NAME##Callback = NAME##Callback;
    PP_CALLBACKS
#undef X
    ppInternal.userdata = userdata;
}

#define X(NAME, ARGS) \
    void pp##NAME##Callback(void(*NAME##Callback)ARGS) { \
        ppInternal.NAME##Callback = NAME##Callback;      \
    }
PP_CALLBACKS
#undef X

#define ppCallCallback(CB, ...)  \
    if (ppInternal.CB##Callback) \
        ppInternal.CB##Callback(ppInternal.userdata, __VA_ARGS__)

void ppUserdata(void *userdata) {
    ppInternal.userdata = userdata;
}

bool ppRunning() {
    return ppInternal.running;
}

#if defined(PP_EMSCRIPTEN)
#include "ppEmscripten.c"
#elif defined(PP_MAC)
#include "ppMac.c"
#elif defined(PP_WINDOWS)
#incldue "ppWindows.c"
#elif defined(PP_LINUX)
#include "ppLinux.c"
#endif
