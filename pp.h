#ifndef pp_h
#define pp_h
#define BITMAP_ENABLE_IO
#define BITMAP_ENABLE_DEBUG_FONT
#define BITMAP_ENABLE_BDF_FONT
#define BITMAP_ENABLE_TTF_FONT
#define BITMAP_ENABLE_GIFS
#include "bitmap.h"
#if defined(_MSC_VER) && _MSC_VER < 1800
#include <windef.h>
#define bool BOOL
#define true 1
#define false 0
#else
#if defined(__STDC__) && __STDC_VERSION__ < 199901L
typedef enum bool { false = 0, true = !false } bool;
#else
#include <stdbool.h>
#endif
#endif

typedef enum {
    ppResizable             = 0x1 << 0,
    ppFullscreen            = 0x1 << 1,
    ppFullscreenDesktop     = 0x1 << 2,
    ppBorderless            = 0x1 << 3,
    ppAlwaysOnTop           = 0x1 << 4
} ppWindowFlags;

typedef enum {
    ppKeyDummy = 0
} ppKey;

typedef enum {
    ppModDummy = 0
} ppKeyMod;

#define PP_CALLBACKS                                \
    X(Keyboard, (void*, ppKey, ppKeyMod, bool))     \
    X(MouseButton, (void*, int, ppKeyMod, bool))    \
    X(MouseMove, (void*, int, int, int, int))       \
    X(MouseScroll, (void*, ppKeyMod, float, float)) \
    X(Focus, (void*, bool))                         \
    X(Resized, (void*, int, int))                   \
    X(Closed, (void*))

bool ppWindow(int w, int h, const char *title, ppWindowFlags flags);
bool ppPollEvents(void);
void ppFlush(Bitmap *bitmap);
void ppRelease(void);

#define X(NAME, ARGS) \
    void(*NAME##Callback)ARGS,
void ppWindowCallbacks(PP_CALLBACKS void *userdata);
#undef X
#define X(NAME, ARGS) \
    void ppWindow##NAME##Callback(void(*NAME##Callback)ARGS);
PP_CALLBACKS
#undef X
void ppWindowUserdata(void *userdata);
bool ppWindowRunning(void);

#endif // pp_h
