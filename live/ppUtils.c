/* ppUtils.h -- https://github.com/takeiteasy/pp
 
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

#include "ppLive.h"
#include <stdlib.h>
#include <assert.h>

ppSurface* ppNewSurface(int w, int h) {
    assert(w && h);
    ppSurface *result = (ppSurface*)malloc(sizeof(ppSurface));
    result->w = w;
    result->h = h;
    result->buf = (int*)malloc(w * h * sizeof(int));
    return result;
}

void ppFreeSurface(ppSurface *surface) {
    assert(surface && surface->buf);
    free(surface->buf);
    free(surface);
}

void ppClearSurface(ppSurface *surface, ppColor color) {
    assert(surface && surface->buf);
    for (int i = 0; i < surface->w * surface->h; i++)
        surface->buf[i] = color.value;
}

#if defined(PP_ENABLE_BLENDING)
#define BLEND(c0, c1, a0, a1) (c0 * a0 / 255) + (c1 * a1 * (255 - a0) / 65025)

static ppColor ppBlendColor(ppColor _a, ppColor _b) {
    return !_a.a ? (ppColor){.value = 0xFF000000} : a >= 255 ? a : (ppColor) {
                                                                       .r = BLEND(_a.r, _b.r, _a.a, _b.a),
                                                                       .g = BLEND(_a.g, _b.g, _a.a, _b.a),
                                                                       .b = BLEND(_a.b, _b.b, _a.a, _b.a),
                                                                       .a = _a.a + (_b.a * (255 - _a.a) >> 8)
                                                                   };
}
#endif

void ppSurfacePSet(ppSurface *surface, int x, int y, ppColor color) {
    assert(surface && surface->buf);
    assert(x >= 0 && y >= 0 && x < surface->w && y < surface->h);
    surface->buf[y * surface->w + x] =
#if defined(PP_ENABLE_BLENDING)
    ppBlendColor(ppPGet(surface, x, y), color);
#else
    color.value;
#endif
}

ppColor ppSurfacePGet(ppSurface *surface, int x, int y) {
    assert(surface && surface->buf);
    assert(x >= 0 && y >= 0 && x < surface->w && y < surface->h);
    return (ppColor){.value = surface->buf[y * surface->w + x]};
}
