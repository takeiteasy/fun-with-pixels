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

static void InitRandom(void) {
    unsigned int seed = (unsigned int)clock();
    for (unsigned int i = 0; i < 17; i++) {
        seed = seed * 2891336453 + 1;
        ppInternal.random.buffer[i] = seed;
    }
    ppInternal.random.p1 = 0;
    ppInternal.random.p2 = 10;
}

#if !defined(PP_LIVE_LIBRARY)
static bool ppBeginNative(int w, int h, const char *title, ppFlags flags);
bool ppBegin(int w, int h, const char *title, ppFlags flags) {
    if (ppInternal.running)
        return false;
    InitRandom();
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
        ppInternal.NAME##Callback = NAME##Callback; \
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
#endif

static unsigned int rotl(unsigned int n, unsigned int r) {
    return (n << r) | (n >> (32 - r));
}

unsigned int ppRandomBits(void) {
#if defined(PP_LIVE_LIBRARY)
    if (!ppInternal.random.p1 && !ppInternal.random.p2)
        InitRandom();
#endif
    unsigned int result = ppInternal.random.buffer[ppInternal.random.p1] = rotl(ppInternal.random.buffer[ppInternal.random.p2], 13) + rotl(ppInternal.random.buffer[ppInternal.random.p1], 9);
    
    if (--ppInternal.random.p1 < 0)
        ppInternal.random.p1 = 16;
    if (--ppInternal.random.p2 < 0)
        ppInternal.random.p2 = 16;
    
    return result;
}

float ppRandomFloat(void) {
    union {
        float value;
        unsigned int word;
    } convert = {
        .word = (ppRandomBits() >> 9) | 0x3f800000};
    return convert.value - 1.0f;
}

double ppRandomDouble(void) {
    unsigned int bits = ppRandomBits();
    union {
        double value;
        unsigned int words[2];
    } convert = {
        .words = {
            bits << 20,
            (bits >> 12) | 0x3FF00000}};
    return convert.value - 1.0;
}

unsigned int ppRandomInt(int max) {
    return ppRandomBits() % max;
}

float ppRandomFloatRange(float min, float max) {
    return ppRandomFloat() * (max - min) + min;
}

double ppRandomDoubleRange(double min, double max) {
    return ppRandomDouble() * (max - min) + min;
}

unsigned int ppRandomIntRange(int min, int max) {
    return ppRandomBits() % (max + 1 - min) + min;
}

int RGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    return ((unsigned int)a << 24) | ((unsigned int)r << 16) | ((unsigned int)g << 8) | b;
}

int RGB(unsigned char r, unsigned char g, unsigned char b) {
    return RGBA(r, g, b, 255);
}

int RGBA1(unsigned char c, unsigned char a) {
    return RGBA(c, c, c, a);
}

int RGB1(unsigned char c) {
    return RGB(c, c, c);
}

unsigned char Rgba(int c) {
    return (unsigned char)((c >> 16) & 0xFF);
}

unsigned char rGba(int c) {
    return (unsigned char)((c >>  8) & 0xFF);
}

unsigned char rgBa(int c) {
    return (unsigned char)(c & 0xFF);
}

unsigned char rgbA(int c) {
    return (unsigned char)((c >> 24) & 0xFF);
}

int rGBA(int c, unsigned char r) {
    return (c & ~0x00FF0000) | (r << 16);
}

int RgBA(int c, unsigned char g) {
    return (c & ~0x0000FF00) | (g << 8);
}

int RGbA(int c, unsigned char b) {
    return (c & ~0x000000FF) | b;
}

int RGBa(int c, unsigned char a) {
    return (c & ~0x00FF0000) | (a << 24);
}

bool InitBitmap(Bitmap *b, unsigned int w, unsigned int h) {
    b->w = w;
    b->h = h;
    size_t sz = w * h * sizeof(unsigned int) + 1;
    b->buf = malloc(sz);
    memset(b->buf, 0, sz);
    return true;
}

void DestroyBitmap(Bitmap *b) {
    if (b->buf)
        free(b->buf);
    memset(b, 0, sizeof(Bitmap));
}

void FillBitmap(Bitmap *b, int col) {
    for (int i = 0; i < b->w * b->h; ++i)
        b->buf[i] = col;
}

static inline void flood_fn(Bitmap *b, int x, int y, int new, int old) {
    if (new == old || PGet(b, x, y) != old)
        return;
    
    int x1 = x;
    while (x1 < b->w && PGet(b, x1, y) == old) {
        PSet(b, x1, y, new);
        x1++;
    }
    
    x1 = x - 1;
    while (x1 >= 0 && PGet(b, x1, y) == old) {
        PSet(b, x1, y, new);
        x1--;
    }
    
    x1 = x;
    while (x1 < b->w && PGet(b, x1, y) == new) {
        if(y > 0 && PGet(b, x1, y - 1) == old)
            flood_fn(b, x1, y - 1, new, old);
        x1++;
    }
    
    x1 = x - 1;
    while(x1 >= 0 && PGet(b, x1, y) == new) {
        if(y > 0 && PGet(b, x1, y - 1) == old)
            flood_fn(b, x1, y - 1, new, old);
        x1--;
    }
    
    x1 = x;
    while(x1 < b->w && PGet(b, x1, y) == new) {
        if(y < b->h - 1 && PGet(b, x1, y + 1) == old)
            flood_fn(b, x1, y + 1, new, old);
        x1++;
    }
    
    x1 = x - 1;
    while(x1 >= 0 && PGet(b, x1, y) == new) {
        if(y < b->h - 1 && PGet(b, x1, y + 1) == old)
            flood_fn(b, x1, y + 1, new, old);
        x1--;
    }
}

void FloodBitmap(Bitmap *b, int x, int y, int col) {
    if (x < 0 || y < 0 || x >= b->w || y >= b->h)
        return;
    flood_fn(b, x, y, col, PGet(b, x, y));
}

void ClearBitmap(Bitmap *b) {
    memset(b->buf, 0, b->w * b->h * sizeof(int));
}

#define BLEND(c0, c1, a0, a1) (c0 * a0 / 255) + (c1 * a1 * (255 - a0) / 65025)

void BSet(Bitmap *b, int x, int y, int c) {
    unsigned char a = rgbA(c);
    if (!a || x < 0 || y < 0 || x >= b->w || y >= b->h)
        return;
    if (a == 255) {
        PSet(b, x, y, c);
        return;
    }
    int *p = &b->buf[y * b->w + x];
    unsigned char _b = rgbA(*p);
    *p = (a == 255 || !b) ? c : RGBA(BLEND(Rgba(c), Rgba(*p), a, _b),
                                     BLEND(rGba(c), rGba(*p), a, _b),
                                     BLEND(rgBa(c), rgBa(*p), a, _b),
                                     a + (_b * (255 - a) >> 8));
}

void PSet(Bitmap *b, int x, int y, int col) {
    if (x >= 0 && y >= 0 && x < b->w && y < b->h)
        b->buf[y * b->w + x] = col;
}

int PGet(Bitmap *b, int x, int y) {
    return (x >= 0 && y >= 0 && x < b->w && y < b->h) ? b->buf[y * b->w + x] : 0;
}

bool PasteBitmap(Bitmap *dst, Bitmap *src, int x, int y) {
    int ox, oy, c;
    for (ox = 0; ox < src->w; ++ox) {
        for (oy = 0; oy < src->h; ++oy) {
            if (oy > dst->h)
                break;
            c = PGet(src, ox, oy);
            BSet(dst, x + ox, y + oy, c);
        }
        if (ox > dst->w)
            break;
    }
    return true;
}

bool PasteBitmapClip(Bitmap *dst, Bitmap *src, int x, int y, int rx, int ry, int rw, int rh) {
    for (int ox = 0; ox < rw; ++ox)
        for (int oy = 0; oy < rh; ++oy)
            BSet(dst, ox + x, oy + y, PGet(src, ox + rx, oy + ry));
    return true;
}

bool CopyBitmap(Bitmap *a, Bitmap *b) {
    if (!InitBitmap(b, a->w, a->h))
        return false;
    memcpy(b->buf, a->buf, a->w * a->h * sizeof(unsigned int) + 1);
    return !!b->buf;
}

void PassthruBitmap(Bitmap *b, int (*fn)(int x, int y, int col)) {
    int x, y;
    for (x = 0; x < b->w; ++x)
        for (y = 0; y < b->h; ++y)
            b->buf[y * b->w + x] = fn(x, y, PGet(b, x, y));
}

bool ScaleBitmap(Bitmap *a, int nw, int nh, Bitmap *b) {
    if (!InitBitmap(b, nw, nh))
        return false;
    
    int x_ratio = (int)((a->w << 16) / b->w) + 1;
    int y_ratio = (int)((a->h << 16) / b->h) + 1;
    int x2, y2, i, j;
    for (i = 0; i < b->h; ++i) {
        int *t = b->buf + i * b->w;
        y2 = ((i * y_ratio) >> 16);
        int *p = a->buf + y2 * a->w;
        int rat = 0;
        for (j = 0; j < b->w; ++j) {
            x2 = (rat >> 16);
            *t++ = p[x2];
            rat += x_ratio;
        }
    }
    return true;
}

#define __MIN(a, b) (((a) < (b)) ? (a) : (b))
#define __MAX(a, b) (((a) > (b)) ? (a) : (b))
#define __D2R(a) ((a) * M_PI / 180.0)

bool RotateBitmap(Bitmap *a, float angle, Bitmap *b) {
    float theta = __D2R(angle);
    float c = cosf(theta), s = sinf(theta);
    float r[3][2] = {
        { -a->h * s, a->h * c },
        {  a->w * c - a->h * s, a->h * c + a->w * s },
        {  a->w * c, a->w * s }
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
    if (!InitBitmap(b, dw, dh))
        return false;
    
    int x, y, sx, sy;
    for (x = 0; x < dw; ++x)
        for (y = 0; y < dh; ++y) {
            sx = ((x + mm[0][0]) * c + (y + mm[0][1]) * s);
            sy = ((y + mm[0][1]) * c - (x + mm[0][0]) * s);
            if (sx < 0 || sx >= a->w || sy < 0 || sy >= a->h)
                continue;
            BSet(b, x, y, PGet(a, sx, sy));
        }
    return true;
}

static inline void vline(Bitmap *b, int x, int y0, int y1, int col) {
    if (y1 < y0) {
        y0 += y1;
        y1  = y0 - y1;
        y0 -= y1;
    }
    
    if (x < 0 || x >= b->w || y0 >= b->h)
        return;
    
    if (y0 < 0)
        y0 = 0;
    if (y1 >= b->h)
        y1 = b->h - 1;
    
    for(int y = y0; y <= y1; y++)
        BSet(b, x, y, col);
}

static inline void hline(Bitmap *b, int y, int x0, int x1, int col) {
    if (x1 < x0) {
        x0 += x1;
        x1  = x0 - x1;
        x0 -= x1;
    }
    
    if (y < 0 || y >= b->h || x0 >= b->w)
        return;
    
    if (x0 < 0)
        x0 = 0;
    if (x1 >= b->w)
        x1 = b->w - 1;
    
    for(int x = x0; x <= x1; x++)
        BSet(b, x, y, col);
}

void DrawLine(Bitmap *b, int x0, int y0, int x1, int y1, int col) {
    if (x0 == x1)
        vline(b, x0, y0, y1, col);
    if (y0 == y1)
        hline(b, y0, x0, x1, col);
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;
    
    while (BSet(b, x0, y0, col), x0 != x1 || y0 != y1) {
        int e2 = err;
        if (e2 > -dx) { err -= dy; x0 += sx; }
        if (e2 <  dy) { err += dx; y0 += sy; }
    }
}

void DrawCircle(Bitmap *b, int xc, int yc, int r, int col, bool fill) {
    int x = -r, y = 0, err = 2 - 2 * r; /* II. Quadrant */
    do {
        BSet(b, xc - x, yc + y, col);    /*   I. Quadrant */
        BSet(b, xc - y, yc - x, col);    /*  II. Quadrant */
        BSet(b, xc + x, yc - y, col);    /* III. Quadrant */
        BSet(b, xc + y, yc + x, col);    /*  IV. Quadrant */
        
        if (fill) {
            hline(b, yc - y, xc - x, xc + x, col);
            hline(b, yc + y, xc - x, xc + x, col);
        }
        
        r = err;
        if (r <= y)
            err += ++y * 2 + 1; /* e_xy+e_y < 0 */
        if (r > x || err > y)
            err += ++x * 2 + 1; /* e_xy+e_x > 0 or no 2nd y-step */
    } while (x < 0);
}

void DrawRect(Bitmap *b, int x, int y, int w, int h, int col, bool fill) {
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
    if (w < 0 || h < 0 || x > b->w || y > b->h)
        return;
    
    if (w > b->w)
        w = b->w;
    if (h > b->h)
        h = b->h;
    
    if (fill) {
        for (; y < h; ++y)
            hline(b, y, x, w, col);
    } else {
        hline(b, y, x, w, col);
        hline(b, h, x, w, col);
        vline(b, x, y, h, col);
        vline(b, w, y, h, col);
    }
}

#define __SWAP(a, b)  \
    do                \
    {                 \
        int temp = a; \
        a = b;        \
        b = temp;     \
    } while (0)

void DrawTri(Bitmap *b, int x0, int y0, int x1, int y1, int x2, int y2, int col, bool fill) {
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
            bool second_half = i > y1 - y0 || y1 == y0;
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
                BSet(b, j, y0 + i, col);
        }
    } else {
        DrawLine(b, x0, y0, x1, y1, col);
        DrawLine(b, x1, y1, x2, y2, col);
        DrawLine(b, x2, y2, x0, y0, col);
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

void DrawCharacter(Bitmap *b, char c, int x, int y, int col) {
    char *bitmap = font8x8_basic[(int)c];
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            PSet(b, x + i, y + j, bitmap[j] & 1 << i ? col : Black);
}

void DrawString(Bitmap *b, const char *str, int x, int y, int col) {
    int nx = x, ny = y;
    for (int i = 0; i < strlen(str); ++i) {
        if (str[i] == '\n') {
            ny += 10;
            nx = x;
        } else {
            DrawCharacter(b, str[i], nx, ny, col);
            nx += 8;
        }
    }
}

void DrawStringFormat(Bitmap *b, int x, int y, int col, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    size_t size = _vscprintf(fmt, args) + 1;
    char *str = malloc(sizeof(char) * size);
    vsnprintf(str, size, fmt, args);
    va_end(args);
    DrawString(b, str, x, y, col);
    free(str);
}

typedef struct {
    const unsigned char *p, *end;
} PNG;

static unsigned get32(const unsigned char* v) {
    return (v[0] << 24) | (v[1] << 16) | (v[2] << 8) | v[3];
}

static const unsigned char* find(PNG* png, const char* chunk, unsigned minlen) {
    const unsigned char* start;
    while (png->p < png->end) {
        unsigned len = get32(png->p + 0);
        start = png->p;
        png->p += len + 12;
        if (memcmp(start + 4, chunk, 4) == 0 && len >= minlen && png->p <= png->end)
            return start + 8;
    }
    return NULL;
}

static unsigned char paeth(unsigned char a, unsigned char b, unsigned char c) {
    int p = a + b - c;
    int pa = abs(p - a), pb = abs(p - b), pc = abs(p - c);
    return (pa <= pb && pa <= pc) ? a : (pb <= pc) ? b : c;
}

static int rowBytes(int w, int bipp) {
    int rowBits = w * bipp;
    return rowBits / 8 + ((rowBits % 8) ? 1 : 0);
}

static int unfilter(int w, int h, int bipp, unsigned char* raw) {
    int len = rowBytes(w, bipp);
    int bpp = rowBytes(1, bipp);
    int x, y;
    unsigned char* first = (unsigned char*)malloc(len + 1);
    memset(first, 0, len + 1);
    unsigned char* prev = first;
    for (y = 0; y < h; y++, prev = raw, raw += len) {
#define LOOP(A, B)            \
    for (x = 0; x < bpp; x++) \
        raw[x] += A;          \
    for (; x < len; x++)      \
        raw[x] += B;          \
    break
        switch (*raw++) {
            case 0:
                break;
            case 1:
                LOOP(0, raw[x - bpp]);
            case 2:
                LOOP(prev[x], prev[x]);
            case 3:
                LOOP(prev[x] / 2, (raw[x - bpp] + prev[x]) / 2);
            case 4:
                LOOP(prev[x], paeth(raw[x - bpp], prev[x], prev[x - bpp]));
            default:
                return 0;
        }
#undef LOOP
    }
    free(first);
    return 1;
}

static void convert(int bypp, int w, int h, const unsigned char* src, int* dest, const unsigned char* trns) {
    int x, y;
    for (y = 0; y < h; y++) {
        src++;  // skip filter byte
        for (x = 0; x < w; x++, src += bypp) {
            switch (bypp) {
                case 1: {
                    unsigned char c = src[0];
                    if (trns && c == *trns) {
                        *dest++ = RGBA1(c, 0);
                        break;
                    } else {
                        *dest++ = RGB1(c);
                        break;
                    }
                }
                case 2:
                    *dest++ = RGBA(src[0], src[0], src[0], src[1]);
                    break;
                case 3: {
                    unsigned char r = src[0];
                    unsigned char g = src[1];
                    unsigned char b = src[2];
                    if (trns && trns[1] == r && trns[3] == g && trns[5] == b) {
                        *dest++ = RGBA(r, g, b, 0);
                        break;
                    } else {
                        *dest++ = RGB(r, g, b);
                        break;
                    }
                }
                case 4:
                    *dest++ = RGBA(src[0], src[1], src[2], src[3]);
                    break;
            }
        }
    }
}

static void depalette(int w, int h, unsigned char* src, int* dest, int bipp, const unsigned char* plte, const unsigned char* trns, int trnsSize) {
    int x, y, c;
    unsigned char alpha;
    int mask, len;

    switch (bipp) {
        case 4:
            mask = 15;
            len = 1;
            break;
        case 2:
            mask = 3;
            len = 3;
            break;
        case 1:
            mask = 1;
            len = 7;
    }

    for (y = 0; y < h; y++) {
        src++;  // skip filter byte
        for (x = 0; x < w; x++) {
            if (bipp == 8) {
                c = *src++;
            } else {
                int pos = x & len;
                c = (src[0] >> ((len - pos) * bipp)) & mask;
                if (pos == len) {
                    src++;
                }
            }
            alpha = 255;
            if (c < trnsSize) {
                alpha = trns[c];
            }
            *dest++ = RGBA(plte[c * 3 + 0], plte[c * 3 + 1], plte[c * 3 + 2], alpha);
        }
    }
}

static int outsize(Bitmap* bmp, int bipp) {
    return (rowBytes(bmp->w, bipp) + 1) * bmp->h;
}

#define PNG_FAIL()      \
    {                   \
        errno = EINVAL; \
        goto err;       \
    }
#define PNG_CHECK(X) \
    if (!(X))        \
        PNG_FAIL()

typedef struct {
    unsigned bits, count;
    const unsigned char *in, *inend;
    unsigned char *out, *outend;
    jmp_buf jmp;
    unsigned litcodes[288], distcodes[32], lencodes[19];
    int tlit, tdist, tlen;
} State;

#define INFLATE_FAIL() longjmp(s->jmp, 1)
#define INFLATE_CHECK(X) \
    if (!(X))            \
        INFLATE_FAIL()

// Built-in DEFLATE standard tables.
static char order[] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };
static char lenBits[29 + 2] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
                                3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 0, 0 };
static int lenBase[29 + 2] = { 3,  4,  5,  6,  7,  8,  9,  10,  11,  13,  15,  17,  19,  23, 27, 31,
                               35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0,  0 };
static char distBits[30 + 2] = { 0, 0, 0, 0, 1, 1, 2,  2,  3,  3,  4,  4,  5,  5,  6, 6,
                                 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 0, 0 };
static int distBase[30 + 2] = {
    1,   2,   3,   4,   5,   7,    9,    13,   17,   25,   33,   49,   65,    97,    129,
    193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577
};

static const unsigned char reverseTable[256] = {
#define R2(n) n, n + 128, n + 64, n + 192
#define R4(n) R2(n), R2(n + 32), R2(n + 16), R2(n + 48)
#define R6(n) R4(n), R4(n + 8), R4(n + 4), R4(n + 12)
    R6(0), R6(2), R6(1), R6(3)
};

static unsigned rev16(unsigned n) {
    return (reverseTable[n & 0xff] << 8) | reverseTable[(n >> 8) & 0xff];
}

static int bits(State* s, int n) {
    int v = s->bits & ((1 << n) - 1);
    s->bits >>= n;
    s->count -= n;
    while (s->count < 16) {
        INFLATE_CHECK(s->in != s->inend);
        s->bits |= (*s->in++) << s->count;
        s->count += 8;
    }
    return v;
}

static unsigned char* emit(State* s, int len) {
    s->out += len;
    INFLATE_CHECK(s->out <= s->outend);
    return s->out - len;
}

static void copy(State* s, const unsigned char* src, int len) {
    unsigned char* dest = emit(s, len);
    while (len--)
        *dest++ = *src++;
}

static int build(State* s, unsigned* tree, unsigned char* lens, int symcount) {
    int n, codes[16], first[16], counts[16] = { 0 };

    // Frequency count.
    for (n = 0; n < symcount; n++)
        counts[lens[n]]++;

    // Distribute codes.
    counts[0] = codes[0] = first[0] = 0;
    for (n = 1; n <= 15; n++) {
        codes[n] = (codes[n - 1] + counts[n - 1]) << 1;
        first[n] = first[n - 1] + counts[n - 1];
    }
    INFLATE_CHECK(first[15] + counts[15] <= symcount);

    // Insert keys into the tree for each symbol.
    for (n = 0; n < symcount; n++) {
        int len = lens[n];
        if (len != 0) {
            int code = codes[len]++, slot = first[len]++;
            tree[slot] = (code << (32 - len)) | (n << 4) | len;
        }
    }

    return first[15];
}

static int decode(State* s, unsigned tree[], int max) {
    // Find the next prefix code.
    unsigned lo = 0, hi = max, key;
    unsigned search = (rev16(s->bits) << 16) | 0xffff;
    while (lo < hi) {
        unsigned guess = (lo + hi) / 2;
        if (search < tree[guess])
            hi = guess;
        else
            lo = guess + 1;
    }

    // Pull out the key and check it.
    key = tree[lo - 1];
    INFLATE_CHECK(((search ^ key) >> (32 - (key & 0xf))) == 0);

    bits(s, key & 0xf);
    return (key >> 4) & 0xfff;
}

static void run(State* s, int sym) {
    int length = bits(s, lenBits[sym]) + lenBase[sym];
    int dsym = decode(s, s->distcodes, s->tdist);
    int offs = bits(s, distBits[dsym]) + distBase[dsym];
    copy(s, s->out - offs, length);
}

static void block(State* s) {
    for (;;) {
        int sym = decode(s, s->litcodes, s->tlit);
        if (sym < 256)
            *emit(s, 1) = (unsigned char)sym;
        else if (sym > 256)
            run(s, sym - 257);
        else
            break;
    }
}

static void stored(State* s) {
    // Uncompressed data block.
    int len;
    bits(s, s->count & 7);
    len = bits(s, 16);
    INFLATE_CHECK(((len ^ s->bits) & 0xffff) == 0xffff);
    INFLATE_CHECK(s->in + len <= s->inend);

    copy(s, s->in, len);
    s->in += len;
    bits(s, 16);
}

static void fixed(State* s) {
    // Fixed set of Huffman codes.
    int n;
    unsigned char lens[288 + 32];
    for (n = 0; n <= 143; n++)
        lens[n] = 8;
    for (n = 144; n <= 255; n++)
        lens[n] = 9;
    for (n = 256; n <= 279; n++)
        lens[n] = 7;
    for (n = 280; n <= 287; n++)
        lens[n] = 8;
    for (n = 0; n < 32; n++)
        lens[288 + n] = 5;

    // Build lit/dist trees.
    s->tlit = build(s, s->litcodes, lens, 288);
    s->tdist = build(s, s->distcodes, lens + 288, 32);
}

static void dynamic(State* s) {
    int n, i, nlit, ndist, nlen;
    unsigned char lenlens[19] = { 0 }, lens[288 + 32];
    nlit = 257 + bits(s, 5);
    ndist = 1 + bits(s, 5);
    nlen = 4 + bits(s, 4);
    for (n = 0; n < nlen; n++)
        lenlens[(unsigned char)order[n]] = (unsigned char)bits(s, 3);

    // Build the tree for decoding code lengths.
    s->tlen = build(s, s->lencodes, lenlens, 19);

    // Decode code lengths.
    for (n = 0; n < nlit + ndist;) {
        int sym = decode(s, s->lencodes, s->tlen);
        switch (sym) {
            case 16:
                for (i = 3 + bits(s, 2); i; i--, n++)
                    lens[n] = lens[n - 1];
                break;
            case 17:
                for (i = 3 + bits(s, 3); i; i--, n++)
                    lens[n] = 0;
                break;
            case 18:
                for (i = 11 + bits(s, 7); i; i--, n++)
                    lens[n] = 0;
                break;
            default:
                lens[n++] = (unsigned char)sym;
                break;
        }
    }

    // Build lit/dist trees.
    s->tlit = build(s, s->litcodes, lens, nlit);
    s->tdist = build(s, s->distcodes, lens + nlit, ndist);
}

static int inflate(void* out, unsigned outlen, const void* in, unsigned inlen) {
    int last;
    State* s = calloc(1, sizeof(State));

    // We assume we can buffer 2 extra bytes from off the end of 'in'.
    s->in = (unsigned char*)in;
    s->inend = s->in + inlen + 2;
    s->out = (unsigned char*)out;
    s->outend = s->out + outlen;
    s->bits = 0;
    s->count = 0;
    bits(s, 0);

    if (setjmp(s->jmp) == 1) {
        free(s);
        return 0;
    }

    do {
        last = bits(s, 1);
        switch (bits(s, 2)) {
            case 0:
                stored(s);
                break;
            case 1:
                fixed(s);
                block(s);
                break;
            case 2:
                dynamic(s);
                block(s);
                break;
            case 3:
                INFLATE_FAIL();
        }
    } while (!last);

    free(s);
    return 1;
}

static bool load_png(Bitmap *b, PNG *png) {
    const unsigned char *ihdr, *idat, *plte, *trns, *first;
    int trnsSize = 0;
    int depth, ctype, bipp;
    int datalen = 0;
    unsigned char *data = NULL, *out;
    
    PNG_CHECK(memcmp(png->p, "\211PNG\r\n\032\n", 8) == 0);  // PNG signature
    png->p += 8;
    first = png->p;
    
    // Read IHDR
    ihdr = find(png, "IHDR", 13);
    PNG_CHECK(ihdr);
    depth = ihdr[8];
    ctype = ihdr[9];
    switch (ctype) {
        case 0:
            bipp = depth;
            break;  // greyscale
        case 2:
            bipp = 3 * depth;
            break;  // RGB
        case 3:
            bipp = depth;
            break;  // paletted
        case 4:
            bipp = 2 * depth;
            break;  // grey+alpha
        case 6:
            bipp = 4 * depth;
            break;  // RGBA
        default:
            PNG_FAIL();
    }
    
    // Allocate bitmap (+1 width to save room for stupid PNG filter bytes)
    InitBitmap(b, get32(ihdr + 0) + 1, get32(ihdr + 4));
    PNG_CHECK(b->buf);
    b->w--;
    
    // We support 8-bit color components and 1, 2, 4 and 8 bit palette formats.
    // No interlacing, or wacky filter types.
    PNG_CHECK((depth != 16) && ihdr[10] == 0 && ihdr[11] == 0 && ihdr[12] == 0);
    
    // Join IDAT chunks.
    for (idat = find(png, "IDAT", 0); idat; idat = find(png, "IDAT", 0)) {
        unsigned len = get32(idat - 8);
        data = realloc(data, datalen + len);
        if (!data)
            break;
        
        memcpy(data + datalen, idat, len);
        datalen += len;
    }
    
    // Find palette.
    png->p = first;
    plte = find(png, "PLTE", 0);
    
    // Find transparency info.
    png->p = first;
    trns = find(png, "tRNS", 0);
    if (trns) {
        trnsSize = get32(trns - 8);
    }
    
    PNG_CHECK(data && datalen >= 6);
    PNG_CHECK((data[0] & 0x0f) == 0x08  // compression method (RFC 1950)
          && (data[0] & 0xf0) <= 0x70   // window size
          && (data[1] & 0x20) == 0);    // preset dictionary present
    
    out = (unsigned char*)b->buf + outsize(b, 32) - outsize(b, bipp);
    PNG_CHECK(inflate(out, outsize(b, bipp), data + 2, datalen - 6));
    PNG_CHECK(unfilter(b->w, b->h, bipp, out));
    
    if (ctype == 3) {
        PNG_CHECK(plte);
        depalette(b->w, b->h, out, b->buf, bipp, plte, trns, trnsSize);
    } else {
        PNG_CHECK(bipp % 8 == 0);
        convert(bipp / 8, b->w, b->h, out, b->buf, trns);
    }
    
    free(data);
    return true;
    
err:
    if (data)
        free(data);
    if (b && b->buf)
        DestroyBitmap(b);
    return false;
}

static unsigned char* read_file(const char *path, size_t *sizeOfFile) {
    FILE *fp = fopen(path, "rb");
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);

    unsigned char *data = malloc(size + 1);
    fread(data, size, 1, fp);
    fclose(fp);
    
    data[size] = '\0';
    *sizeOfFile = size;
    return data;
}

bool LoadBitmap(Bitmap *out, const char *path) {
    unsigned char *data = NULL;
    size_t sizeOfData = 0;
    if (!(data = read_file(path, &sizeOfData)) && sizeOfData > 0)
        return false;
    bool result = LoadBitmapMemory(out, (void*)data, sizeOfData);
    free(data);
    return result;
}

bool LoadBitmapMemory(Bitmap *out, const void *data, size_t sizeOfData) {
    PNG png = {
        .p = (unsigned char*)data,
        .end = (unsigned char*)data + sizeOfData
    };
    return load_png(out, &png);
}

typedef struct {
    unsigned crc, adler, bits, prev, runlen;
    FILE* out;
    unsigned crcTable[256];
} Save;

static const unsigned crctable[16] = { 0,          0x1db71064, 0x3b6e20c8, 0x26d930ac, 0x76dc4190, 0x6b6b51f4,
                                       0x4db26158, 0x5005713c, 0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
                                       0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c };

static void put(Save* s, unsigned v) {
    fputc(v, s->out);
    s->crc = (s->crc >> 4) ^ crctable[(s->crc & 15) ^ (v & 15)];
    s->crc = (s->crc >> 4) ^ crctable[(s->crc & 15) ^ (v >> 4)];
}

static void updateAdler(Save* s, unsigned v) {
    unsigned s1 = s->adler & 0xffff, s2 = (s->adler >> 16) & 0xffff;
    s1 = (s1 + v) % 65521;
    s2 = (s2 + s1) % 65521;
    s->adler = (s2 << 16) + s1;
}

static void put32(Save* s, unsigned v) {
    put(s, (v >> 24) & 0xff);
    put(s, (v >> 16) & 0xff);
    put(s, (v >> 8) & 0xff);
    put(s, v & 0xff);
}

void putbits(Save* s, unsigned data, unsigned bitcount) {
    while (bitcount--) {
        unsigned prev = s->bits;
        s->bits = (s->bits >> 1) | ((data & 1) << 7);
        data >>= 1;
        if (prev & 1) {
            put(s, s->bits);
            s->bits = 0x80;
        }
    }
}

void putbitsr(Save* s, unsigned data, unsigned bitcount) {
    while (bitcount--)
        putbits(s, data >> bitcount, 1);
}

static void begin(Save* s, const char* id, unsigned len) {
    put32(s, len);
    s->crc = 0xffffffff;
    put(s, id[0]);
    put(s, id[1]);
    put(s, id[2]);
    put(s, id[3]);
}

static void literal(Save* s, unsigned v) {
    // Encode a literal/length using the built-in tables.
    // Could do better with a custom table but whatever.
    if (v < 144)
        putbitsr(s, 0x030 + v - 0, 8);
    else if (v < 256)
        putbitsr(s, 0x190 + v - 144, 9);
    else if (v < 280)
        putbitsr(s, 0x000 + v - 256, 7);
    else
        putbitsr(s, 0x0c0 + v - 280, 8);
}

static void encodelen(Save* s, unsigned code, unsigned bits, unsigned len) {
    literal(s, code + (len >> bits));
    putbits(s, len, bits);
    putbits(s, 0, 5);
}

static void endrun(Save* s) {
    s->runlen--;
    literal(s, s->prev);

    if (s->runlen >= 67)
        encodelen(s, 277, 4, s->runlen - 67);
    else if (s->runlen >= 35)
        encodelen(s, 273, 3, s->runlen - 35);
    else if (s->runlen >= 19)
        encodelen(s, 269, 2, s->runlen - 19);
    else if (s->runlen >= 11)
        encodelen(s, 265, 1, s->runlen - 11);
    else if (s->runlen >= 3)
        encodelen(s, 257, 0, s->runlen - 3);
    else
        while (s->runlen--)
            literal(s, s->prev);
}

static void encodeByte(Save* s, unsigned char v) {
    updateAdler(s, v);

    // Simple RLE compression. We could do better by doing a search
    // to find matches, but this works pretty well TBH.
    if (s->prev == v && s->runlen < 115) {
        s->runlen++;
    } else {
        if (s->runlen)
            endrun(s);

        s->prev = v;
        s->runlen = 1;
    }
}

static void savePngHeader(Save* s, Bitmap* bmp) {
    fwrite("\211PNG\r\n\032\n", 8, 1, s->out);
    begin(s, "IHDR", 13);
    put32(s, bmp->w);
    put32(s, bmp->h);
    put(s, 8);  // bit depth
    put(s, 6);  // RGBA
    put(s, 0);  // compression (deflate)
    put(s, 0);  // filter (standard)
    put(s, 0);  // interlace off
    put32(s, ~s->crc);
}

static long savePngData(Save* s, Bitmap* bmp, long dataPos) {
    int x, y;
    long dataSize;
    begin(s, "IDAT", 0);
    put(s, 0x08);      // zlib compression method
    put(s, 0x1d);      // zlib compression flags
    putbits(s, 3, 3);  // zlib last block + fixed dictionary
    for (y = 0; y < bmp->h; y++) {
        int *row = &bmp->buf[y * bmp->w];
        int prev = RGBA1(0, 0);

        encodeByte(s, 1);  // sub filter
        for (x = 0; x < bmp->w; x++) {
            encodeByte(s, Rgba(row[x]) - Rgba(prev));
            encodeByte(s, rGba(row[x]) - rGba(prev));
            encodeByte(s, rgBa(row[x]) - rgBa(prev));
            encodeByte(s, rgbA(row[x]) - rgbA(prev));
            prev = row[x];
        }
    }
    endrun(s);
    literal(s, 256);  // terminator
    while (s->bits != 0x80)
        putbits(s, 0, 1);
    put32(s, s->adler);
    dataSize = (ftell(s->out) - dataPos) - 8;
    put32(s, ~s->crc);
    return dataSize;
}

bool SaveBitmap(Bitmap *b, const char *path) {
    FILE* out = fopen(path, "wb");
    if (!out)
        return false;
    
    Save s;
    s.out = out;
    s.adler = 1;
    s.bits = 0x80;
    s.prev = 0xffff;
    s.runlen = 0;
    
    savePngHeader(&s, b);
    long dataPos = ftell(s.out);
    long dataSize = savePngData(&s, b, dataPos);
    
    // End chunk.
    begin(&s, "IEND", 0);
    put32(&s, ~s.crc);
    
    // Write back payload size.
    fseek(out, dataPos, SEEK_SET);
    put32(&s, (int)dataSize);
    
    long err = ferror(out);
    fclose(out);
    return !err;
}

bool FlipBitmapHorizontal(Bitmap *a, Bitmap *b) {
    if (!InitBitmap(b, a->w, a->h))
        return false;
    for (int y = 0; y < a->h; y++)
        for (int x = a->w - 1, i = 0; x >= 0; x--, i++)
            PSet(b, i, y, PGet(a, x, y));
    return true;
}

bool FlipBitmapVertical(Bitmap *a, Bitmap *b) {
    if (!InitBitmap(b, a->w, a->h))
        return false;
    for (int x = 0; x < a->w; x++)
        for (int y = a->h - 1, i = 0; y >= 0; y--, i++)
            PSet(b, x, i, PGet(a, x, y));
    return true;
}

bool GenBitmapCheckerboard(Bitmap *b, int w, int h, int cw, int ch, int col1, int col2) {
    if (!InitBitmap(b, w, h))
        return false;
    for (int x = 0; x < w; x++)
        for (int y = 0; y < h; y++)
            PSet(b, x, y, (x / cw + y / ch) % 2 == 0 ? col1 : col2);
    return true;
}

#define GRADIENT(A, B, F) (int)((float)(B) * (F) + (float)(A)*(1.f - (F)))
#define PSET_GRADIENT(B, X, Y, F, C1, C2) \
    PSet((B), (X), (y), RGBA(GRADIENT(Rgba((C1)), Rgba((C2)), (F)), \
                             GRADIENT(rGba((C1)), rGba((C2)), (F)), \
                             GRADIENT(rgBa((C1)), rgBa((C2)), (F)), \
                             GRADIENT(rgbA((C1)), rgbA((C2)), (F))));

bool GenBitmapGradientHorizontal(Bitmap *b, int w, int h, int col1, int col2) {
    if (!InitBitmap(b, w, h))
        return false;
    for (int x = 0; x < w; x++) {
        float f = x / w;
        for (int y = 0; y < h; y++)
            PSET_GRADIENT(b, x, y, f, col1, col2);
    }
    return true;
}

bool GenBitmapGradientVertical(Bitmap *b, int w, int h, int col1, int col2) {
    if (!InitBitmap(b, w, h))
        return false;
    for (int y = 0; y < h; y++) {
        float f = y / h;
        for (int x = 0; x < w; x++)
            PSET_GRADIENT(b, x, y, f, col1, col2);
    }
    return true;
}

static float fclamp(float f, float min, float max) {
    return fmaxf(fminf(f, max), min);
}

bool GenBitmapGradientRadial(Bitmap *b, int w, int h, float d, int col1, int col2) {
    if (!InitBitmap(b, w, h))
        return false;
    
    float cx = (float)w / 2.f;
    float cy = (float)h / 2.f;
    float r = w < h ? cx : cy;
    for (int x = 0; x < w; x++)
        for (int y = 0; y < h; y++) {
            float f = fclamp((hypotf((float)x - cx, (float)y - cy) - r * d)/(r * (1.0f - d)), 0.f, 1.f);
            PSET_GRADIENT(b, x, y, f, col1, col2);
        }
    return true;
}

bool GenBitmapWhiteNoise(Bitmap *b, int w, int h, float factor) {
    if (!InitBitmap(b, w, h))
        return false;
    for (int i = 0; i < w * h; i++)
        b->buf[i] = ppRandomInt(99) < (int)(factor * 100.f) ? White : Black;
    return true;
}

static const float grad3[][3] = {
    { 1, 1, 0 }, { -1, 1, 0 }, { 1, -1, 0 }, { -1, -1, 0 },
    { 1, 0, 1 }, { -1, 0, 1 }, { 1, 0, -1 }, { -1, 0, -1 },
    { 0, 1, 1 }, { 0, -1, 1 }, { 0, 1, -1 }, { 0, -1, -1 }
};

static const unsigned int perm[] = {
    182, 232, 51, 15, 55, 119, 7, 107, 230, 227, 6, 34, 216, 61, 183, 36,
    40, 134, 74, 45, 157, 78, 81, 114, 145, 9, 209, 189, 147, 58, 126, 0,
    240, 169, 228, 235, 67, 198, 72, 64, 88, 98, 129, 194, 99, 71, 30, 127,
    18, 150, 155, 179, 132, 62, 116, 200, 251, 178, 32, 140, 130, 139, 250, 26,
    151, 203, 106, 123, 53, 255, 75, 254, 86, 234, 223, 19, 199, 244, 241, 1,
    172, 70, 24, 97, 196, 10, 90, 246, 252, 68, 84, 161, 236, 205, 80, 91,
    233, 225, 164, 217, 239, 220, 20, 46, 204, 35, 31, 175, 154, 17, 133, 117,
    73, 224, 125, 65, 77, 173, 3, 2, 242, 221, 120, 218, 56, 190, 166, 11,
    138, 208, 231, 50, 135, 109, 213, 187, 152, 201, 47, 168, 185, 186, 167, 165,
    102, 153, 156, 49, 202, 69, 195, 92, 21, 229, 63, 104, 197, 136, 148, 94,
    171, 93, 59, 149, 23, 144, 160, 57, 76, 141, 96, 158, 163, 219, 237, 113,
    206, 181, 112, 111, 191, 137, 207, 215, 13, 83, 238, 249, 100, 131, 118, 243,
    162, 248, 43, 66, 226, 27, 211, 95, 214, 105, 108, 101, 170, 128, 210, 87,
    38, 44, 174, 188, 176, 39, 14, 143, 159, 16, 124, 222, 33, 247, 37, 245,
    8, 4, 22, 82, 110, 180, 184, 12, 25, 5, 193, 41, 85, 177, 192, 253,
    79, 29, 115, 103, 142, 146, 52, 48, 89, 54, 121, 212, 122, 60, 28, 42,
    
    182, 232, 51, 15, 55, 119, 7, 107, 230, 227, 6, 34, 216, 61, 183, 36,
    40, 134, 74, 45, 157, 78, 81, 114, 145, 9, 209, 189, 147, 58, 126, 0,
    240, 169, 228, 235, 67, 198, 72, 64, 88, 98, 129, 194, 99, 71, 30, 127,
    18, 150, 155, 179, 132, 62, 116, 200, 251, 178, 32, 140, 130, 139, 250, 26,
    151, 203, 106, 123, 53, 255, 75, 254, 86, 234, 223, 19, 199, 244, 241, 1,
    172, 70, 24, 97, 196, 10, 90, 246, 252, 68, 84, 161, 236, 205, 80, 91,
    233, 225, 164, 217, 239, 220, 20, 46, 204, 35, 31, 175, 154, 17, 133, 117,
    73, 224, 125, 65, 77, 173, 3, 2, 242, 221, 120, 218, 56, 190, 166, 11,
    138, 208, 231, 50, 135, 109, 213, 187, 152, 201, 47, 168, 185, 186, 167, 165,
    102, 153, 156, 49, 202, 69, 195, 92, 21, 229, 63, 104, 197, 136, 148, 94,
    171, 93, 59, 149, 23, 144, 160, 57, 76, 141, 96, 158, 163, 219, 237, 113,
    206, 181, 112, 111, 191, 137, 207, 215, 13, 83, 238, 249, 100, 131, 118, 243,
    162, 248, 43, 66, 226, 27, 211, 95, 214, 105, 108, 101, 170, 128, 210, 87,
    38, 44, 174, 188, 176, 39, 14, 143, 159, 16, 124, 222, 33, 247, 37, 245,
    8, 4, 22, 82, 110, 180, 184, 12, 25, 5, 193, 41, 85, 177, 192, 253,
    79, 29, 115, 103, 142, 146, 52, 48, 89, 54, 121, 212, 122, 60, 28, 42
};


static float dot3(const float a[], float x, float y, float z) {
    return a[0]*x + a[1]*y + a[2]*z;
}

static float lerp(float a, float b, float t) {
    return (1 - t) * a + t * b;
}

static float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

#if !defined(PP_SIMPLEX_NOISE) && !defined(PP_PERLIN_NOISE)
#define PP_SIMPLEX_NOISE
#endif
#if defined(PP_SIMPLEX_NOISE) && defined(PP_PERLIN_NOISE)
#undef PP_PERLIN_NOISE
#endif

#if defined(PP_SIMPLEX_NOISE)
#define FASTFLOOR(x)  (((x) >= 0) ? (int)(x) : (int)(x)-1)

static float noise(float x, float y, float z) {
    /* Skew input space */
    float s = (x+y+z)*(1.0/3.0);
    int i = FASTFLOOR(x+s);
    int j = FASTFLOOR(y+s);
    int k = FASTFLOOR(z+s);
    
    /* Unskew */
    float t = (float)(i+j+k)*(1.0/6.0);
    float gx0 = i-t;
    float gy0 = j-t;
    float gz0 = k-t;
    float x0 = x-gx0;
    float y0 = y-gy0;
    float z0 = z-gz0;
    
    /* Determine simplex */
    int i1, j1, k1;
    int i2, j2, k2;
    
    if (x0 >= y0) {
        if (y0 >= z0) {
            i1 = 1; j1 = 0; k1 = 0;
            i2 = 1; j2 = 1; k2 = 0;
        } else if (x0 >= z0) {
            i1 = 1; j1 = 0; k1 = 0;
            i2 = 1; j2 = 0; k2 = 1;
        } else {
            i1 = 0; j1 = 0; k1 = 1;
            i2 = 1; j2 = 0; k2 = 1;
        }
    } else {
        if (y0 < z0) {
            i1 = 0; j1 = 0; k1 = 1;
            i2 = 0; j2 = 1; k2 = 1;
        } else if (x0 < z0) {
            i1 = 0; j1 = 1; k1 = 0;
            i2 = 0; j2 = 1; k2 = 1;
        } else {
            i1 = 0; j1 = 1; k1 = 0;
            i2 = 1; j2 = 1; k2 = 0;
        }
    }
    
    /* Calculate offsets in x,y,z coords */
    float x1 = x0 - i1 + (1.0/6.0);
    float y1 = y0 - j1 + (1.0/6.0);
    float z1 = z0 - k1 + (1.0/6.0);
    float x2 = x0 - i2 + 2.0*(1.0/6.0);
    float y2 = y0 - j2 + 2.0*(1.0/6.0);
    float z2 = z0 - k2 + 2.0*(1.0/6.0);
    float x3 = x0 - 1.0 + 3.0*(1.0/6.0);
    float y3 = y0 - 1.0 + 3.0*(1.0/6.0);
    float z3 = z0 - 1.0 + 3.0*(1.0/6.0);
    
    int ii = i % 256;
    int jj = j % 256;
    int kk = k % 256;
    
    /* Calculate gradient incides */
    int gi0 = perm[ii+perm[jj+perm[kk]]] % 12;
    int gi1 = perm[ii+i1+perm[jj+j1+perm[kk+k1]]] % 12;
    int gi2 = perm[ii+i2+perm[jj+j2+perm[kk+k2]]] % 12;
    int gi3 = perm[ii+1+perm[jj+1+perm[kk+1]]] % 12;
    
    /* Calculate contributions */
    float n0, n1, n2, n3;
    
    float t0 = 0.6 - x0*x0 - y0*y0 - z0*z0;
    if (t0 < 0)
        n0 = 0.0;
    else {
        t0 *= t0;
        n0 = t0 * t0 * dot3(grad3[gi0], x0, y0, z0);
    }
    
    float t1 = 0.6 - x1*x1 - y1*y1 - z1*z1;
    if (t1 < 0)
        n1 = 0.0;
    else {
        t1 *= t1;
        n1 = t1 * t1 * dot3(grad3[gi1], x1, y1, z1);
    }
    
    float t2 = 0.6 - x2*x2 - y2*y2 - z2*z2;
    if (t2 < 0)
        n2 = 0.0;
    else {
        t2 *= t2;
        n2 = t2 * t2 * dot3(grad3[gi2], x2, y2, z2);
    }
    
    float t3 = 0.6 - x3*x3 - y3*y3 - z3*z3;
    if (t3 < 0)
        n3 = 0.0;
    else {
        t3 *= t3;
        n3 = t3 * t3 * dot3(grad3[gi3], x3, y3, z3);
    }
    
    /* Return scaled sum of contributions */
    return 32.0*(n0 + n1 + n2 + n3);
}
#endif

#if defined(PP_PERLIN_NOISE)
static float noise(float x, float y, float z) {
    /* Find grid points */
    int gx = FASTFLOOR(x);
    int gy = FASTFLOOR(y);
    int gz = FASTFLOOR(z);
    
    /* Relative coords within grid cell */
    float rx = x - gx;
    float ry = y - gy;
    float rz = z - gz;
    
    /* Wrap cell coords */
    gx = gx & 255;
    gy = gy & 255;
    gz = gz & 255;
    
    /* Calculate gradient indices */
    unsigned int gi[8];
    for (int i = 0; i < 8; i++)
        gi[i] = perm[gx+((i>>2)&1)+perm[gy+((i>>1)&1)+perm[gz+(i&1)]]] % 12;
    
    /* Noise contribution from each corner */
    float n[8];
    for (int i = 0; i < 8; i++)
        n[i] = dot3(grad3[gi[i]], rx - ((i>>2)&1), ry - ((i>>1)&1), rz - (i&1));
    
    /* Fade curves */
    float u = fade(rx);
    float v = fade(ry);
    float w = fade(rz);
    
    /* Interpolate */
    float nx[4];
    for (int i = 0; i < 4; i++)
        nx[i] = lerp(n[i], n[4+i], u);
    
    float nxy[2];
    for (int i = 0; i < 2; i++)
        nxy[i] = lerp(nx[i], nx[2+i], v);
    
    return lerp(nxy[0], nxy[1], w);
}
#endif

static float invlerp(float a, float b, float v) {
    return (v - a) / (b - a);
}

static float remap(float imin, float imax, float omin, float omax, float v) {
    return lerp(omin, omax, invlerp(imin, imax, v));
}

bool GenBitmapFBMNoise(Bitmap *b, int w, int h, int offsetX, int offsetY, float scale, float lacunarity, float gain, int octaves) {
    if (!InitBitmap(b, w, h))
        return false;
    
    float max = FLT_MIN;
    float min = FLT_MAX;
    float *map = malloc(sizeof(float) * w * h);
    if (!map)
        return false;
    
    for (int x = 0; x < w; x++)
        for (int y = 0; y < h; y++) {
            float amplitude = 1.f;
            float frequency = 1.f;
            float p = 0.f;
            
            for (int i = 0; i < octaves; i++) {
                float nx = (float)(x + offsetX) * scale / (float)w;
                float ny = (float)(y + offsetY) * scale / (float)h;
                float n  = noise(nx, ny, 1.f) * 2 - 1;
                p += n * amplitude;
                
                amplitude *= gain;
                frequency *= lacunarity;
            }
            
            if (p > max)
                max = p;
            if (p < min)
                min = p;
            map[y * w + x] = p;
        }
    
    for (int x = 0; x < w; x++)
        for (int y = 0; y < h; y++) {
            int c = (int)remap(min, max, 0.f, 255.f, map[y * w + x]);
            PSet(b, x, y, RGB1(c));
        }
    
    free(map);
    return true;
}

#if defined(PP_LIVE) && !defined(PP_LIVE_LIBRARY)
#if defined(PP_WINDOWS)
#include <Windows.h>
#include <io.h>
#define F_OK    0
#define access _access
#include "getopt.h"
#ifndef _MSC_VER
#pragma comment(lib, "Psapi.lib")
#endif
#include "dlfcn.h"
FILETIME writeTime;
#else
#include <getopt.h>
#define _BSD_SOURCE // usleep()
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>
static ino_t handleID;
#endif

static void *handle = NULL;
static ppState *state = NULL;
static ppApp *app = NULL;
static struct {
    int width;
    int height;
    const char *title;
    ppFlags flags;
    int clearColor;
    char *path;
} Args = {0};
static Bitmap pbo;

static struct option long_options[] = {
    {"width", required_argument, NULL, 'w'},
    {"height", required_argument, NULL, 'h'},
    {"title", required_argument, NULL, 't'},
    {"resizable", no_argument, NULL, 'r'},
    {"top", no_argument, NULL, 'a'},
    {"usage", no_argument, NULL, 'u'},
    {"path", required_argument, NULL, 'p'},
    {NULL, 0, NULL, 0}
};

static void usage(void) {
    puts("usage: pp -p [path] [options]\n");
    puts("\t-w/--width\tWindow width\t\t [default: 640]");
    puts("\t-h/--height\tWindow height\t\t [default: 480]");
    puts("\t-t/--title\tWindow title\t\t [default: \"pp\"]");
    puts("\t-r/--resizable\tEnable resizable window");
    puts("\t-a/--top\tEnable window always on top");
    puts("\t-p/--path\tPath the dynamic library [required]");
    puts("\t-u/--usage\tDisplay this message");
}

#if defined(PP_WINDOWS)
static FILETIME Win32GetLastWriteTime(char* path) {
    FILETIME time;
    WIN32_FILE_ATTRIBUTE_DATA data;

    if (GetFileAttributesEx(path, GetFileExInfoStandard, &data))
        time = data.ftLastWriteTime;

    return time;
}
#endif

static bool ShouldReloadLibrary(void) {
#if defined(PP_WINDOWS)
    FILETIME newTime = Win32GetLastWriteTime(Args.path);
    bool result = CompareFileTime(&newTime, &writeTime);
    if (result)
        writeTime = newTime;
    return result;
#else
    struct stat attr;
    bool result = !stat(Args.path, &attr) && handleID != attr.st_ino;
    if (result)
        handleID = attr.st_ino;
    return result;
#endif
}

#if defined(PP_WINDOWS)
char* RemoveExt(char* path) {
    char *ret = malloc(strlen(path) + 1);
    if (!ret)
        return NULL;
    strcpy(ret, path);
    char *ext = strrchr(ret, '.');
    if (ext)
        *ext = '\0';
    return ret;
}
#endif

static bool ReloadLibrary(const char *path) {
    if (!ShouldReloadLibrary())
        return true;
    
    if (handle) {
        if (app->unload)
            app->unload(state);
        dlclose(handle);
    }
    
#if defined(PP_WINDOWS)
    size_t newPathSize = strlen(path) + 4;
    char *newPath = malloc(sizeof(char) * newPathSize);
    char *noExt = RemoveExt(path);
    sprintf(newPath, "%s.tmp.dll", noExt);
    CopyFile(path, newPath, 0);
    handle = dlopen(newPath, RTLD_NOW);
    free(newPath);
    free(noExt);
    if (!handle)
#else
    if (!(handle = dlopen(path, RTLD_NOW)))
#endif
        goto BAIL;
    if (!(app = dlsym(handle, "pp")))
        goto BAIL;
    if (!state) {
        if (!(state = app->init()))
            goto BAIL;
    } else {
        if (app->reload)
            app->reload(state);
    }
    return true;
    
BAIL:
    if (handle)
        dlclose(handle);
    handle = NULL;
#if defined(PP_WINDOWS)
    memset(&writeTime, 0, sizeof(FILETIME));
#else
    handleID = 0;
#endif
    return false;
}

#define ppInputCallback(E)      \
    if (app->event)             \
        app->event(state, &(E)) \

static void ppInputKeyboard(void *userdata, ppKey key, ppMod modifier, bool isDown) {
    ppEvent e = {
        .type = KeyboardEvent,
        .Keyboard = {
            .key = key,
            .isdown = isDown
        },
        .modifier = modifier
    };
    ppInputCallback(e);
}

static void ppInputMouseButton(void *userdata, int button, ppMod modifier, bool isDown) {
    ppEvent e = {
        .type = MouseButtonEvent,
        .Mouse = {
            .button = button,
            .isdown = isDown
        },
        .modifier = modifier
    };
    ppInputCallback(e);
}

static void ppInputMouseMove(void *userdata, int x, int y, float dx, float dy) {
    ppEvent e = {
        .type = MouseMoveEvent,
        .Mouse = {
            .Position = {
                .x = x,
                .y = y,
                .dx = dx,
                .dy = dy
            }
        }
    };
    ppInputCallback(e);
}

static void ppInputMouseScroll(void *userdata, float dx, float dy, ppMod modifier) {
    ppEvent e = {
        .type = MouseScrollEvent,
        .Mouse = {
            .Scroll = {
                .dx = dx,
                .dy = dy
            }
        }
    };
    ppInputCallback(e);
}

static void ppInputFocus(void *userdata, bool isFocused) {
    ppEvent e = {
        .type = FocusEvent,
        .Window = {
            .focused = isFocused
        }
    };
    ppInputCallback(e);
}

static void ppInputResized(void *userdata, int w, int h) {
    ppEvent e = {
        .type = ResizedEvent,
        .Window = {
            .Size = {
                .width = w,
                .height = h
            }
        }
    };
    ppInputCallback(e);
}

static void ppInputClosed(void *userdata) {
    ppEvent e = {
        .type = ClosedEvent,
        .Window = {
            .closed = true
        }
    };
    ppInputCallback(e);
}

int main(int argc, char *argv[]) {
    extern char* optarg;
    extern int optopt;
    int opt;
    while ((opt = getopt_long(argc, argv, ":w:h:t:p:uar", long_options, NULL)) != -1) {
        switch (opt) {
            case 'w':
                Args.width = atoi(optarg);
                break;
            case 'h':
                Args.height = atoi(optarg);
                break;
            case 't':
                Args.title = optarg;
                break;
            case 'r':
                Args.flags |= ppResizable;
                break;
            case 'a':
                Args.flags |= ppAlwaysOnTop;
                break;
            case 'p':
                Args.path = optarg;
                break;
            case ':':
                printf("ERROR: \"-%c\" requires an value!\n", optopt);
                usage();
                return EXIT_FAILURE;
            case '?':
                printf("ERROR: Unknown argument \"-%c\"\n", optopt);
                usage();
                return EXIT_FAILURE;
            case 'u':
                usage();
                return EXIT_SUCCESS;
            default:
                usage();
                return EXIT_FAILURE;
        }
    }
    
    if (!Args.path) {
        puts("ERROR: No path to dynamic library provided (-p/--path)");
        usage();
        return EXIT_FAILURE;
    } else {
#if !defined(PP_WINDOWS)
        if (Args.path[0] != '.' || Args.path[1] != '/') {
            char *tmp = malloc(strlen(Args.path) + 2 * sizeof(char));
            sprintf(tmp, "./%s", Args.path);
            Args.path = tmp;
        } else
            Args.path = strdup(Args.path);
#endif
    }

    if (access(Args.path, F_OK)) {
        printf("ERROR: No file found at path \"%s\"\n", Args.path);
        return EXIT_FAILURE;
    }
    
    if (!Args.width)
        Args.width = 640;
    if (!Args.height)
        Args.height = 480;
    if (!Args.clearColor)
        Args.clearColor = Black;
    ppBegin(Args.width, Args.height, Args.title ? Args.title : "pp", Args.flags);
    InitBitmap(&pbo, Args.width, Args.height);
    
    if (!ReloadLibrary(Args.path))
        return EXIT_FAILURE;
    
#define X(NAME, _) ppInput##NAME,
    ppCallbacks(PP_CALLBACKS NULL);
#undef X

    while (ppPoll()) {
        if (!ReloadLibrary(Args.path))
            break;
        if (!app->tick(state, &pbo, ppTime()))
            break;
        ppFlush(&pbo);
    }
    
    app->deinit(state);
    if (handle)
        dlclose(handle);
    DestroyBitmap(&pbo);
#if !defined(PP_WINDOWS)
    free(Args.path);
#endif
    ppEnd();
    return EXIT_SUCCESS;
}
#endif
