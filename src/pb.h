/* pb.h -- https://github.com/takeiteasy/fwp
 
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

#ifndef FWP_PB_HEADER
#define FWP_PB_HEADER
#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <math.h>

#if defined(_WIN32) || defined(_WIN64)
#define FWP_PB_WINDOWS
#endif

#if defined(FWP_PB_WINDOWS) && !defined(FWP_PB_NO_EXPORT)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

typedef int32_t pbColor;

typedef enum {
    IndianRed = -3318692,
    LightCoral = -1015680,
    Salmon = -360334,
    DarkSalmon = -1468806,
    LightSalmon = -24454,
    Crimson = -2354116,
    Red = -65536,
    FireBrick = -5103070,
    DarkRed = -7667712,
    Pink = -16181,
    LightPink = -18751,
    HotPink = -38476,
    DeepPink = -60269,
    MediumVioletRed = -3730043,
    PaleVioletRed = -2396013,
    Coral = -32944,
    Tomato = -40121,
    OrangeRed = -47872,
    DarkOrange = -29696,
    Orange = -23296,
    Gold = -10496,
    Yellow = -256,
    LightYellow = -32,
    LemonChiffon = -1331,
    LightGoldenrodYellow = -329006,
    PapayaWhip = -4139,
    Moccasin = -6987,
    PeachPuff = -9543,
    PaleGoldenrod = -1120086,
    Khaki = -989556,
    DarkKhaki = -4343957,
    Lavender = -1644806,
    Thistle = -2572328,
    Plum = -2252579,
    Violet = -1146130,
    Orchid = -2461482,
    Fuchsia = -65281,
    Magenta = -65281,
    MediumOrchid = -4565549,
    MediumPurple = -7114533,
    RebeccaPurple = -10079335,
    BlueViolet = -7722014,
    DarkViolet = -7077677,
    DarkOrchid = -6737204,
    DarkMagenta = -7667573,
    Purple = -8388480,
    Indigo = -11861886,
    SlateBlue = -9807155,
    DarkSlateBlue = -12042869,
    MediumSlateBlue = -8689426,
    GreenYellow = -5374161,
    Chartreuse = -8388864,
    LawnGreen = -8586240,
    Lime = -16711936,
    LimeGreen = -13447886,
    PaleGreen = -6751336,
    LightGreen = -7278960,
    MediumSpringGreen = -16713062,
    SpringGreen = -16711809,
    MediumSeaGreen = -12799119,
    SeaGreen = -13726889,
    ForestGreen = -14513374,
    Green = -16744448,
    DarkGreen = -16751616,
    YellowGreen = -6632142,
    OliveDrab = -9728477,
    Olive = -8355840,
    DarkOliveGreen = -11179217,
    MediumAquamarine = -10039894,
    DarkSeaGreen = -7357301,
    LightSeaGreen = -14634326,
    DarkCyan = -16741493,
    Teal = -16744320,
    Aqua = -16711681,
    Cyan = -16711681,
    LightCyan = -2031617,
    PaleTurquoise = -5247250,
    Aquamarine = -8388652,
    Turquoise = -12525360,
    MediumTurquoise = -12004916,
    DarkTurquoise = -16724271,
    CadetBlue = -10510688,
    SteelBlue = -12156236,
    LightSteelBlue = -5192482,
    PowderBlue = -5185306,
    LightBlue = -5383962,
    SkyBlue = -7876885,
    LightSkyBlue = -7876870,
    DeepSkyBlue = -16728065,
    DodgerBlue = -14774017,
    CornflowerBlue = -10185235,
    RoyalBlue = -12490271,
    Blue = -16776961,
    MediumBlue = -16777011,
    DarkBlue = -16777077,
    Navy = -16777088,
    MidnightBlue = -15132304,
    Cornsilk = -1828,
    BlanchedAlmond = -5171,
    Bisque = -6972,
    NavajoWhite = -8531,
    Wheat = -663885,
    BurlyWood = -2180985,
    Tan = -2968436,
    RosyBrown = -4419697,
    SandyBrown = -744352,
    Goldenrod = -2448096,
    DarkGoldenrod = -4684277,
    Peru = -3308225,
    Chocolate = -2987746,
    SaddleBrown = -7650029,
    Sienna = -6270419,
    Brown = -5952982,
    Maroon = -8388608,
    White = -1,
    Snow = -1286,
    HoneyDew = -983056,
    MintCream = -655366,
    Azure = -983041,
    AliceBlue = -984833,
    GhostWhite = -460545,
    WhiteSmoke = -657931,
    SeaShell = -2578,
    Beige = -657956,
    OldLace = -133658,
    FloralWhite = -1296,
    Ivory = -16,
    AntiqueWhite = -332841,
    Linen = -331546,
    LavenderBlush = -3851,
    MistyRose = -6943,
    Gainsboro = -2302756,
    LightGray = -2894893,
    Silver = -4144960,
    DarkGray = -5658199,
    Gray = -8355712,
    DimGray = -9868951,
    LightSlateGray = -8943463,
    SlateGray = -9404272,
    DarkSlateGray = -13676721,
    Black = -16777216,
} pbBuiltinColor;

EXPORT pbColor RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
EXPORT pbColor RGB(uint8_t r, uint8_t g, uint8_t b);
EXPORT pbColor RGBA1(uint8_t c, uint8_t a);
EXPORT pbColor RGB1(uint8_t c);

EXPORT uint8_t Rgba(pbColor c);
EXPORT uint8_t rGba(pbColor c);
EXPORT uint8_t rgBa(pbColor c);
EXPORT uint8_t rgbA(pbColor c);

EXPORT pbColor rGBA(pbColor c, uint8_t r);
EXPORT pbColor RgBA(pbColor c, uint8_t g);
EXPORT pbColor RGbA(pbColor c, uint8_t b);
EXPORT pbColor RGBa(pbColor c, uint8_t a);

typedef struct {
    unsigned int width, height;
    pbColor *buffer;
} pbImage;

EXPORT pbImage* pbImageNew(unsigned int w, unsigned int h);
EXPORT void pbImageFree(pbImage *img);

EXPORT void pbImageFill(pbImage *img, pbColor col);
EXPORT void pbImageFlood(pbImage *img, int x, int y, pbColor col);
EXPORT void pbImagePSet(pbImage *img, int x, int y, pbColor col);
EXPORT pbColor pbImagePGet(pbImage *img, int x, int y);
EXPORT void pbImagePaste(pbImage *dst, pbImage *src, int x, int y);
EXPORT void pbImagePartialPaste(pbImage *dst, pbImage *src, int x, int y, int rx, int ry, int rw, int rh);
EXPORT pbImage* pbImageDupe(pbImage *src);
EXPORT void pbImagePassThru(pbImage *img, pbColor(*fn)(int x, int y, pbColor col));
EXPORT pbImage* pbImageResize(pbImage *src, int nw, int nh);
EXPORT pbImage* pbImageRotate(pbImage *src, float angle);
EXPORT void pbImageDrawLine(pbImage *img, int x0, int y0, int x1, int y1, pbColor col);
EXPORT void pbImageDrawCircle(pbImage *img, int xc, int yc, int r, pbColor col, int fill);
EXPORT void pbImageDrawRectangle(pbImage *img, int x, int y, int w, int h, pbColor col, int fill);
EXPORT void pbImageDrawTriangle(pbImage *img, int x0, int y0, int x1, int y1, int x2, int y2, pbColor col, int fill);

EXPORT void pbImageDrawCharacter(pbImage *img, char c, int x, int y, pbColor col);
EXPORT void pbImageDrawString(pbImage *img, const char *str, int x, int y, pbColor col);
EXPORT void pbImageDrawStringFormat(pbImage *img, int x, int y, pbColor col, const char *fmt, ...);

EXPORT pbImage* pbImageLoadFromPath(const char *path);
EXPORT pbImage* pbImageLoadFromMemory(const void *data, size_t length);
EXPORT int pbImageSave(pbImage *img, const char *path);

typedef enum {
    pbResizable         = 1 << 0,
    pbFullscreen        = 1 << 1,
    pbFullscreenDesktop = 1 << 2,
    pbBorderless        = 1 << 3,
    pbAlwaysOnTop       = 1 << 4
} pbFlags;

typedef enum {
    KEY_PAD0=128,KEY_PAD1,KEY_PAD2,KEY_PAD3,KEY_PAD4,KEY_PAD5,KEY_PAD6,KEY_PAD7,
    KEY_PAD8,KEY_PAD9,KEY_PADMUL,KEY_PADADD,KEY_PADENTER,KEY_PADSUB,KEY_PADDOT,
    KEY_PADDIV,KEY_F1,KEY_F2,KEY_F3,KEY_F4,KEY_F5,KEY_F6,KEY_F7,KEY_F8,KEY_F9,
    KEY_F10,KEY_F11,KEY_F12,KEY_BACKSPACE,KEY_TAB,KEY_RETURN,KEY_SHIFT,
    KEY_CONTROL,KEY_ALT,KEY_PAUSE,KEY_CAPSLOCK,KEY_ESCAPE,KEY_SPACE,KEY_PAGEUP,
    KEY_PAGEDN,KEY_END,KEY_HOME,KEY_LEFT,KEY_UP,KEY_RIGHT,KEY_DOWN,KEY_INSERT,
    KEY_DELETE,KEY_LWIN,KEY_RWIN,KEY_NUMLOCK,KEY_SCROLL,KEY_LSHIFT,KEY_RSHIFT,
    KEY_LCONTROL,KEY_RCONTROL,KEY_LALT,KEY_RALT,KEY_SEMICOLON,KEY_EQUALS,
    KEY_COMMA,KEY_MINUS,KEY_DOT,KEY_SLASH,KEY_BACKTICK,KEY_LSQUARE,
    KEY_BACKSLASH,KEY_RSQUARE,KEY_TICK
} pbKey;

typedef enum {
    KEY_MOD_SHIFT     = 1 << 0,
    KEY_MOD_CONTROL   = 1 << 1,
    KEY_MOD_ALT       = 1 << 2,
    KEY_MOD_SUPER     = 1 << 3,
    KEY_MOD_CAPS_LOCK = 1 << 4,
    KEY_MOD_NUM_LOCK  = 1 << 5
} pbMod;

#define FWP_PB_CALLBACKS                             \
    X(Keyboard,     (void*, int, int, int))          \
    X(MouseButton,  (void*, int, int, int))          \
    X(MouseMove,    (void*, int, int, float, float)) \
    X(MouseScroll,  (void*, float, float, int))      \
    X(Resized,      (void*, int, int))               \
    X(Focus,        (void*, int))                    \
    X(Closed,       (void*))

EXPORT int pbBegin(unsigned int w, unsigned int h, const char *title, pbFlags flags);
EXPORT int pbPoll(void);
EXPORT void pbFlush(pbImage *buffer);
EXPORT void pbEnd(void);

EXPORT void pbSetWindowTitle(const char *title);
EXPORT void pbWindowSize(unsigned int *w, unsigned int *h);
EXPORT void pbSetWindowSize(unsigned int w, unsigned int h);
EXPORT void pbCursorPosition(int *x, int *y);

#define X(NAME, ARGS) \
    void(*NAME##Callback)ARGS,
EXPORT void pbCallbacks(FWP_PB_CALLBACKS void *userdata);
#undef X
#define X(NAME, ARGS) \
    EXPORT void pb##NAME##Callback(void(*NAME##Callback)ARGS);
FWP_PB_CALLBACKS
#undef X

EXPORT void pbUserdata(void *userdata);
EXPORT int pbRunning(void);

#if defined(__cplusplus)
}
#endif
#endif // FWP_PB_HEADER

#if defined(FWP_PB_IMPLEMENTATION)
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

pbColor RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return ((uint8_t)a << 24) | ((uint8_t)r << 16) | ((uint8_t)g << 8) | b;
}

pbColor RGB(uint8_t r, uint8_t g, uint8_t b) {
    return RGBA(r, g, b, 255);
}

pbColor RGBA1(uint8_t c, uint8_t a) {
    return RGBA(c, c, c, a);
}

pbColor RGB1(uint8_t c) {
    return RGB(c, c, c);
}

uint8_t Rgba(pbColor c) {
    return (uint8_t)((c >> 16) & 0xFF);
}

uint8_t rGba(pbColor c) {
    return (uint8_t)((c >>  8) & 0xFF);
}

uint8_t rgBa(pbColor c) {
    return (uint8_t)(c & 0xFF);
}

uint8_t rgbA(pbColor c) {
    return (uint8_t)((c >> 24) & 0xFF);
}

pbColor rGBA(pbColor c, uint8_t r) {
    return (c & ~0x00FF0000) | (r << 16);
}

pbColor RgBA(pbColor c, uint8_t g) {
    return (c & ~0x0000FF00) | (g << 8);
}

pbColor RGbA(pbColor c, uint8_t b) {
    return (c & ~0x000000FF) | b;
}

pbColor RGBa(pbColor c, uint8_t a) {
    return (c & ~0x00FF0000) | (a << 24);
}

pbImage* pbImageNew(unsigned int w, unsigned int h) {
    pbImage *result = malloc(sizeof(pbImage));
    result->width = w;
    result->height = h;
    result->buffer = malloc(w * h * sizeof(int));
    return result;
}

void pbImageFree(pbImage *img) {
    if (img) {
        if (img->buffer)
            free(img->buffer);
        free(img);
    }
}

void pbImageFill(pbImage *img, pbColor col) {
    for (int i = 0; i < img->width * img->height; ++i)
        img->buffer[i] = col;
}

static inline void flood_fn(pbImage *img, int x, int y, pbColor new, pbColor old) {
    if (new == old || pbImagePGet(img, x, y) != old)
        return;
    
    int x1 = x;
    while (x1 < img->width && pbImagePGet(img, x1, y) == old) {
        pbImagePSet(img, x1, y, new);
        x1++;
    }
    
    x1 = x - 1;
    while (x1 >= 0 && pbImagePGet(img, x1, y) == old) {
        pbImagePSet(img, x1, y, new);
        x1--;
    }
    
    x1 = x;
    while (x1 < img->width && pbImagePGet(img, x1, y) == new) {
        if(y > 0 && pbImagePGet(img, x1, y - 1) == old)
            flood_fn(img, x1, y - 1, new, old);
        x1++;
    }
    
    x1 = x - 1;
    while(x1 >= 0 && pbImagePGet(img, x1, y) == new) {
        if(y > 0 && pbImagePGet(img, x1, y - 1) == old)
            flood_fn(img, x1, y - 1, new, old);
        x1--;
    }
    
    x1 = x;
    while(x1 < img->width && pbImagePGet(img, x1, y) == new) {
        if(y < img->height - 1 && pbImagePGet(img, x1, y + 1) == old)
            flood_fn(img, x1, y + 1, new, old);
        x1++;
    }
    
    x1 = x - 1;
    while(x1 >= 0 && pbImagePGet(img, x1, y) == new) {
        if(y < img->height - 1 && pbImagePGet(img, x1, y + 1) == old)
            flood_fn(img, x1, y + 1, new, old);
        x1--;
    }
}

void pbImageFlood(pbImage *img, int x, int y, pbColor col) {
    if (x < 0 || y < 0 || x >= img->width || y >= img->height)
        return;
    flood_fn(img, x, y, col, pbImagePGet(img, x, y));
}

#define BLEND(c0, c1, a0, a1) (c0 * a0 / 255) + (c1 * a1 * (255 - a0) / 65025)

static int Blend(pbColor _a, pbColor _b) {
    int a   = rgbA(_a);
    int b   = rgbA(_b);
    return !a ? 0xFF000000 : a >= 255 ? RGBa(a, 255) : RGBA(BLEND(Rgba(_a), Rgba(_b), a, b),
                                                            BLEND(rGba(_a), rGba(_b), a, b),
                                                            BLEND(rgBa(_a), rgBa(_b), a, b),
                                                            a + (b * (255 - a) >> 8));
}

void pbImagePSet(pbImage *img, int x, int y, pbColor col) {
    if (x >= 0 && y >= 0 && x < img->width && y < img->height) {
        int a = rgbA(col);
        img->buffer[y * img->width + x] = a == 255 ? col : a == 0 ? 0 : Blend(pbImagePGet(img, x, y), col);
    }
}

int pbImagePGet(pbImage *img, int x, int y) {
    return (x >= 0 && y >= 0 && x < img->width && y < img->height) ? img->buffer[y * img->width + x] : 0;
}

void pbImagePaste(pbImage *dst, pbImage *src, int x, int y) {
    for (int ox = 0; ox < src->width; ++ox) {
        for (int oy = 0; oy < src->height; ++oy) {
            if (oy > dst->height)
                break;
            pbImagePSet(dst, x + ox, y + oy, pbImagePGet(src, ox, oy));
        }
        if (ox > dst->width)
            break;
    }
}

void pbImagePartialPaste(pbImage *dst, pbImage *src, int x, int y, int rx, int ry, int rw, int rh) {
    for (int ox = 0; ox < rw; ++ox)
        for (int oy = 0; oy < rh; ++oy)
            pbImagePSet(dst, ox + x, oy + y, pbImagePGet(src, ox + rx, oy + ry));
}

pbImage* pbImageDupe(pbImage *src) {
    pbImage *result = pbImageNew(src->width, src->height);
    memcpy(result->buffer, src->buffer, src->width * src->height * sizeof(int));
    return result;
}

void pbImagePassThru(pbImage *img, pbColor(*fn)(int x, int y, pbColor col)) {
    int x, y;
    for (x = 0; x < img->width; ++x)
        for (y = 0; y < img->height; ++y)
            img->buffer[y * img->width + x] = fn(x, y, pbImagePGet(img, x, y));
}

pbImage* pbImageResize(pbImage *src, int nw, int nh) {
    pbImage *result = pbImageNew(nw, nh);
    int x_ratio = (int)((src->width << 16) / result->width) + 1;
    int y_ratio = (int)((src->height << 16) / result->height) + 1;
    int x2, y2, i, j;
    for (i = 0; i < result->height; ++i) {
        int *t = result->buffer + i * result->width;
        y2 = ((i * y_ratio) >> 16);
        int *p = src->buffer + y2 * src->width;
        int rat = 0;
        for (j = 0; j < result->width; ++j) {
            x2 = (rat >> 16);
            *t++ = p[x2];
            rat += x_ratio;
        }
    }
    return result;
}

#define __MIN(a, b) (((a) < (b)) ? (a) : (b))
#define __MAX(a, b) (((a) > (b)) ? (a) : (b))
#define __D2R(a) ((a) * M_PI / 180.0)

pbImage* pbImageRotate(pbImage *src, float angle) {
    float theta = __D2R(angle);
    float c = cosf(theta), s = sinf(theta);
    float r[3][2] = {
        { -src->height * s, src->height * c },
        {  src->width * c - src->height * s, src->height * c + src->width * s },
        {  src->width * c, src->width * s }
    };
    
    float mm[2][2] = {{
        __MIN(0, __MIN(r[0][0], __MIN(r[1][0], r[2][0]))),
        __MIN(0, __MIN(r[0][1], __MIN(r[1][1], r[2][1])))
    }, {
        (theta > 1.5708  && theta < 3.14159 ? 0.f : __MAX(r[0][0], __MAX(r[1][0], r[2][0]))),
        (theta > 3.14159 && theta < 4.71239 ? 0.f : __MAX(r[0][1], __MAX(r[1][1], r[2][1])))
    }};
    
    int dw = (int)ceil(fabsf(mm[1][0]) - mm[0][0]);
    int dh = (int)ceil(fabsf(mm[1][1]) - mm[0][1]);
    pbImage *result = pbImageNew(dw, dh);
    
    int x, y, sx, sy;
    for (x = 0; x < dw; ++x)
        for (y = 0; y < dh; ++y) {
            sx = ((x + mm[0][0]) * c + (y + mm[0][1]) * s);
            sy = ((y + mm[0][1]) * c - (x + mm[0][0]) * s);
            if (sx < 0 || sx >= src->width || sy < 0 || sy >= src->height)
                continue;
            pbImagePSet(result, x, y, pbImagePGet(src, sx, sy));
        }
    return result;
}

static inline void vline(pbImage *img, int x, int y0, int y1, pbColor col) {
    if (y1 < y0) {
        y0 += y1;
        y1  = y0 - y1;
        y0 -= y1;
    }
    
    if (x < 0 || x >= img->width || y0 >= img->height)
        return;
    
    if (y0 < 0)
        y0 = 0;
    if (y1 >= img->height)
        y1 = img->height - 1;
    
    for(int y = y0; y <= y1; y++)
        pbImagePSet(img, x, y, col);
}

static inline void hline(pbImage *img, int y, int x0, int x1, pbColor col) {
    if (x1 < x0) {
        x0 += x1;
        x1  = x0 - x1;
        x0 -= x1;
    }
    
    if (y < 0 || y >= img->height || x0 >= img->width)
        return;
    
    if (x0 < 0)
        x0 = 0;
    if (x1 >= img->width)
        x1 = img->width - 1;
    
    for(int x = x0; x <= x1; x++)
        pbImagePSet(img, x, y, col);
}

void pbImageDrawLine(pbImage *img, int x0, int y0, int x1, int y1, pbColor col) {
    if (x0 == x1)
        vline(img, x0, y0, y1, col);
    if (y0 == y1)
        hline(img, y0, x0, x1, col);
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;
    
    while (pbImagePSet(img, x0, y0, col), x0 != x1 || y0 != y1) {
        int e2 = err;
        if (e2 > -dx) { err -= dy; x0 += sx; }
        if (e2 <  dy) { err += dx; y0 += sy; }
    }
}

void pbImageDrawCircle(pbImage *img, int xc, int yc, int r, pbColor col, int fill) {
    int x = -r, y = 0, err = 2 - 2 * r; /* II. Quadrant */
    do {
        pbImagePSet(img, xc - x, yc + y, col);    /*   I. Quadrant */
        pbImagePSet(img, xc - y, yc - x, col);    /*  II. Quadrant */
        pbImagePSet(img, xc + x, yc - y, col);    /* III. Quadrant */
        pbImagePSet(img, xc + y, yc + x, col);    /*  IV. Quadrant */
        
        if (fill) {
            hline(img, yc - y, xc - x, xc + x, col);
            hline(img, yc + y, xc - x, xc + x, col);
        }
        
        r = err;
        if (r <= y)
            err += ++y * 2 + 1; /* e_xy+e_y < 0 */
        if (r > x || err > y)
            err += ++x * 2 + 1; /* e_xy+e_x > 0 or no 2nd y-step */
    } while (x < 0);
}

void pbImageDrawRectangle(pbImage *img, int x, int y, int w, int h, pbColor col, int fill) {
    if (x < 0) {
        w += x;
        x  = 0;
    }
    if (y < 0) {
        h += y;
        y  = 0;
    }
    
    w += x;
    h += y;
    if (w < 0 || h < 0 || x > img->width || y > img->height)
        return;
    
    if (w > img->width)
        w = img->width;
    if (h > img->height)
        h = img->height;
    
    if (fill) {
        for (; y < h; ++y)
            hline(img, y, x, w, col);
    } else {
        hline(img, y, x, w, col);
        hline(img, h, x, w, col);
        vline(img, x, y, h, col);
        vline(img, w, y, h, col);
    }
}

#define __SWAP(a, b)  \
    do                \
    {                 \
        int temp = a; \
        a = b;        \
        b = temp;     \
    } while (0)

void pbImageDrawTriangle(pbImage *img, int x0, int y0, int x1, int y1, int x2, int y2, pbColor col, int fill) {
    if (y0 ==  y1 && y0 ==  y2)
        return;
    if (fill) {
        if (y0 > y1) {
            __SWAP(x0, x1);
            __SWAP(y0, y1);
        }
        if (y0 > y2) {
            __SWAP(x0, x2);
            __SWAP(y0, y2);
        }
        if (y1 > y2) {
            __SWAP(x1, x2);
            __SWAP(y1, y2);
        }
        
        int total_height = y2 - y0, i, j;
        for (i = 0; i < total_height; ++i) {
            int second_half = i > y1 - y0 || y1 == y0;
            int segment_height = second_half ? y2 - y1 : y1 - y0;
            float alpha = (float)i / total_height;
            float beta  = (float)(i - (second_half ? y1 - y0 : 0)) / segment_height;
            int ax = x0 + (x2 - x0) * alpha;
            int ay = y0 + (y2 - y0) * alpha;
            int bx = second_half ? x1 + (x2 - x1) : x0 + (x1 - x0) * beta;
            int by = second_half ? y1 + (y2 - y1) : y0 + (y1 - y0) * beta;
            if (ax > bx) {
                __SWAP(ax, bx);
                __SWAP(ay, by);
            }
            for (j = ax; j <= bx; ++j)
                pbImagePSet(img, j, y0 + i, col);
        }
    } else {
        pbImageDrawLine(img, x0, y0, x1, y1, col);
        pbImageDrawLine(img, x1, y1, x2, y2, col);
        pbImageDrawLine(img, x2, y2, x0, y0, col);
    }
}

#if !defined(_WIN32) && !defined(_WIN64)
// Taken from: https://stackoverflow.com/a/4785411
static int _vscprintf(const char *format, va_list pargs) {
    va_list argcopy;
    va_copy(argcopy, pargs);
    int retval = vsnprintf(NULL, 0, format, argcopy);
    va_end(argcopy);
    return retval;
}
#endif

static char font8x8_basic[128][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0000 (nul)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0001
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0002
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0003
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0004
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0005
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0006
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0007
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0008
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0009
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+000A
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+000B
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+000C
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+000D
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+000E
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+000F
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0010
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0011
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0012
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0013
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0014
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0015
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0016
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0017
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0018
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0019
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+001A
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+001B
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+001C
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+001D
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+001E
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+001F
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0020 (space)
    {0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00}, // U+0021 (!)
    {0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0022 (")
    {0x36, 0x36, 0x7F, 0x36, 0x7F, 0x36, 0x36, 0x00}, // U+0023 (#)
    {0x0C, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x0C, 0x00}, // U+0024 ($)
    {0x00, 0x63, 0x33, 0x18, 0x0C, 0x66, 0x63, 0x00}, // U+0025 (%)
    {0x1C, 0x36, 0x1C, 0x6E, 0x3B, 0x33, 0x6E, 0x00}, // U+0026 (&)
    {0x06, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0027 (')
    {0x18, 0x0C, 0x06, 0x06, 0x06, 0x0C, 0x18, 0x00}, // U+0028 (()
    {0x06, 0x0C, 0x18, 0x18, 0x18, 0x0C, 0x06, 0x00}, // U+0029 ())
    {0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00}, // U+002A (*)
    {0x00, 0x0C, 0x0C, 0x3F, 0x0C, 0x0C, 0x00, 0x00}, // U+002B (+)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x06}, // U+002C (,)
    {0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00}, // U+002D (-)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00}, // U+002E (.)
    {0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00}, // U+002F (/)
    {0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00}, // U+0030 (0)
    {0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00}, // U+0031 (1)
    {0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00}, // U+0032 (2)
    {0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00}, // U+0033 (3)
    {0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00}, // U+0034 (4)
    {0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00}, // U+0035 (5)
    {0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00}, // U+0036 (6)
    {0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00}, // U+0037 (7)
    {0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00}, // U+0038 (8)
    {0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00}, // U+0039 (9)
    {0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x00}, // U+003A (:)
    {0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x06}, // U+003B (;)
    {0x18, 0x0C, 0x06, 0x03, 0x06, 0x0C, 0x18, 0x00}, // U+003C (<)
    {0x00, 0x00, 0x3F, 0x00, 0x00, 0x3F, 0x00, 0x00}, // U+003D (=)
    {0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00}, // U+003E (>)
    {0x1E, 0x33, 0x30, 0x18, 0x0C, 0x00, 0x0C, 0x00}, // U+003F (?)
    {0x3E, 0x63, 0x7B, 0x7B, 0x7B, 0x03, 0x1E, 0x00}, // U+0040 (@)
    {0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00}, // U+0041 (A)
    {0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00}, // U+0042 (B)
    {0x3C, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3C, 0x00}, // U+0043 (C)
    {0x1F, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1F, 0x00}, // U+0044 (D)
    {0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F, 0x00}, // U+0045 (E)
    {0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x0F, 0x00}, // U+0046 (F)
    {0x3C, 0x66, 0x03, 0x03, 0x73, 0x66, 0x7C, 0x00}, // U+0047 (G)
    {0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00}, // U+0048 (H)
    {0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // U+0049 (I)
    {0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E, 0x00}, // U+004A (J)
    {0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00}, // U+004B (K)
    {0x0F, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7F, 0x00}, // U+004C (L)
    {0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x00}, // U+004D (M)
    {0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x00}, // U+004E (N)
    {0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00}, // U+004F (O)
    {0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F, 0x00}, // U+0050 (P)
    {0x1E, 0x33, 0x33, 0x33, 0x3B, 0x1E, 0x38, 0x00}, // U+0051 (Q)
    {0x3F, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x67, 0x00}, // U+0052 (R)
    {0x1E, 0x33, 0x07, 0x0E, 0x38, 0x33, 0x1E, 0x00}, // U+0053 (S)
    {0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // U+0054 (T)
    {0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3F, 0x00}, // U+0055 (U)
    {0x33, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00}, // U+0056 (V)
    {0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00}, // U+0057 (W)
    {0x63, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x00}, // U+0058 (X)
    {0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E, 0x00}, // U+0059 (Y)
    {0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F, 0x00}, // U+005A (Z)
    {0x1E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x1E, 0x00}, // U+005B ([)
    {0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x40, 0x00}, // U+005C (\)
    {0x1E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1E, 0x00}, // U+005D (])
    {0x08, 0x1C, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00}, // U+005E (^)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF}, // U+005F (_)
    {0x0C, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0060 (`)
    {0x00, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x6E, 0x00}, // U+0061 (a)
    {0x07, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3B, 0x00}, // U+0062 (b)
    {0x00, 0x00, 0x1E, 0x33, 0x03, 0x33, 0x1E, 0x00}, // U+0063 (c)
    {0x38, 0x30, 0x30, 0x3e, 0x33, 0x33, 0x6E, 0x00}, // U+0064 (d)
    {0x00, 0x00, 0x1E, 0x33, 0x3f, 0x03, 0x1E, 0x00}, // U+0065 (e)
    {0x1C, 0x36, 0x06, 0x0f, 0x06, 0x06, 0x0F, 0x00}, // U+0066 (f)
    {0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x1F}, // U+0067 (g)
    {0x07, 0x06, 0x36, 0x6E, 0x66, 0x66, 0x67, 0x00}, // U+0068 (h)
    {0x0C, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // U+0069 (i)
    {0x30, 0x00, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E}, // U+006A (j)
    {0x07, 0x06, 0x66, 0x36, 0x1E, 0x36, 0x67, 0x00}, // U+006B (k)
    {0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // U+006C (l)
    {0x00, 0x00, 0x33, 0x7F, 0x7F, 0x6B, 0x63, 0x00}, // U+006D (m)
    {0x00, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x00}, // U+006E (n)
    {0x00, 0x00, 0x1E, 0x33, 0x33, 0x33, 0x1E, 0x00}, // U+006F (o)
    {0x00, 0x00, 0x3B, 0x66, 0x66, 0x3E, 0x06, 0x0F}, // U+0070 (p)
    {0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x78}, // U+0071 (q)
    {0x00, 0x00, 0x3B, 0x6E, 0x66, 0x06, 0x0F, 0x00}, // U+0072 (r)
    {0x00, 0x00, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x00}, // U+0073 (s)
    {0x08, 0x0C, 0x3E, 0x0C, 0x0C, 0x2C, 0x18, 0x00}, // U+0074 (t)
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x6E, 0x00}, // U+0075 (u)
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00}, // U+0076 (v)
    {0x00, 0x00, 0x63, 0x6B, 0x7F, 0x7F, 0x36, 0x00}, // U+0077 (w)
    {0x00, 0x00, 0x63, 0x36, 0x1C, 0x36, 0x63, 0x00}, // U+0078 (x)
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x3E, 0x30, 0x1F}, // U+0079 (y)
    {0x00, 0x00, 0x3F, 0x19, 0x0C, 0x26, 0x3F, 0x00}, // U+007A (z)
    {0x38, 0x0C, 0x0C, 0x07, 0x0C, 0x0C, 0x38, 0x00}, // U+007B ({)
    {0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00}, // U+007C (|)
    {0x07, 0x0C, 0x0C, 0x38, 0x0C, 0x0C, 0x07, 0x00}, // U+007D (})
    {0x6E, 0x3B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+007E (~)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  // U+007F
};

void pbImageDrawCharacter(pbImage *img, char c, int x, int y, pbColor col) {
    char *bitmap = font8x8_basic[(int)c];
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            pbImagePSet(img, x + i, y + j, bitmap[j] & 1 << i ? col : 0xFF000000);
}

void pbImageDrawString(pbImage *img, const char *str, int x, int y, pbColor col) {
    int nx = x, ny = y;
    for (int i = 0; i < strlen(str); ++i) {
        if (str[i] == '\n') {
            ny += 10;
            nx = x;
        } else {
            pbImageDrawCharacter(img, str[i], nx, ny, col);
            nx += 8;
        }
    }
}

void pbImageDrawStringFormat(pbImage *img, int x, int y, pbColor col, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    size_t size = _vscprintf(fmt, args) + 1;
    char *str = malloc(sizeof(char) * size);
    vsnprintf(str, size, fmt, args);
    va_end(args);
    pbImageDrawString(img, str, x, y, col);
    free(str);
}


static struct {
#define X(NAME, ARGS) void(*NAME##Callback)ARGS;
    FWP_PB_CALLBACKS
#undef X
    void *userdata;
    int running;
    int *data, w, h;
    unsigned int windowWidth;
    unsigned int windowHeight;
    int cursorX;
    int cursorY;
} pbInternal = {0};

int pbBeginNative(int w, int h, const char *title, pbFlags flags);
int pbPollNative(void);
void pbFlushNative(pbImage *buffer);
void pbEndNative(void);
void pbSetWindowSizeNative(unsigned int w, unsigned int h);
void pbSetWindowTitleNative(const char *title);

int pbBegin(unsigned int w, unsigned int h, const char *title, pbFlags flags) {
    assert(!pbInternal.running);
    pbInternal.data = (void*)0;
    pbInternal.w = 0;
    pbInternal.h = 0;
    pbInternal.running = pbBeginNative(w, h, title, flags);
    return pbInternal.running;
}

int pbPoll(void) {
    return pbPollNative();
}

void pbFlush(pbImage *buffer) {
    pbFlushNative(buffer);
}

void pbEnd(void) {
    pbEndNative();
}

void pbSetWindowTitle(const char *title) {
    pbSetWindowTitleNative(title);
}

void pbWindowSize(unsigned int *w, unsigned int *h) {
    if (w)
        *w = pbInternal.windowWidth;
    if (h)
        *h = pbInternal.windowHeight;
}

void pbSetWindowSize(unsigned int w, unsigned int h) {
    pbSetWindowSizeNative(w, h);
}

void pbCursorPosition(int *x, int *y) {
    if (x)
        *x = pbInternal.cursorX;
    if (y)
        *y = pbInternal.cursorY;
}

#define X(NAME, ARGS) \
    void(*NAME##Callback)ARGS,
void pbCallbacks(FWP_PB_CALLBACKS void* userdata) {
#undef X
#define X(NAME, ARGS) \
    pbInternal.NAME##Callback = NAME##Callback;
    FWP_PB_CALLBACKS
#undef X
    pbInternal.userdata = userdata;
}

#define X(NAME, ARGS)                                    \
    void pb##NAME##Callback(void(*NAME##Callback)ARGS) { \
        pbInternal.NAME##Callback = NAME##Callback;      \
    }
FWP_PB_CALLBACKS
#undef X

#define pbCallCallback(CB, ...)  \
    if (pbInternal.CB##Callback) \
        pbInternal.CB##Callback(pbInternal.userdata, __VA_ARGS__)

void pbUserdata(void *userdata) {
    pbInternal.userdata = userdata;
}

int pbRunning(void) {
    return pbInternal.running;
}
#endif
