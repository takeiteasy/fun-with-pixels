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
    ppResizable             = 1 << 0,
    ppFullscreen            = 1 << 1,
    ppFullscreenDesktop     = 1 << 2,
    ppBorderless            = 1 << 3,
    ppAlwaysOnTop           = 1 << 4
} ppFlags;

typedef enum {
    KEY_PAD0=128,KEY_PAD1,KEY_PAD2,KEY_PAD3,KEY_PAD4,KEY_PAD5,KEY_PAD6,KEY_PAD7,KEY_PAD8,KEY_PAD9,
    KEY_PADMUL,KEY_PADADD,KEY_PADENTER,KEY_PADSUB,KEY_PADDOT,KEY_PADDIV,
    KEY_F1,KEY_F2,KEY_F3,KEY_F4,KEY_F5,KEY_F6,KEY_F7,KEY_F8,KEY_F9,KEY_F10,KEY_F11,KEY_F12,
    KEY_BACKSPACE,KEY_TAB,KEY_RETURN,KEY_SHIFT,KEY_CONTROL,KEY_ALT,KEY_PAUSE,KEY_CAPSLOCK,
    KEY_ESCAPE,KEY_SPACE,KEY_PAGEUP,KEY_PAGEDN,KEY_END,KEY_HOME,KEY_LEFT,KEY_UP,KEY_RIGHT,KEY_DOWN,
    KEY_INSERT,KEY_DELETE,KEY_LWIN,KEY_RWIN,KEY_NUMLOCK,KEY_SCROLL,KEY_LSHIFT,KEY_RSHIFT,
    KEY_LCONTROL,KEY_RCONTROL,KEY_LALT,KEY_RALT,KEY_SEMICOLON,KEY_EQUALS,KEY_COMMA,KEY_MINUS,
    KEY_DOT,KEY_SLASH,KEY_BACKTICK,KEY_LSQUARE,KEY_BACKSLASH,KEY_RSQUARE,KEY_TICK
} ppKey;

typedef enum {
    KEY_MOD_SHIFT   = 1 << 0,
    KEY_MOD_CONTROL = 1 << 1,
    KEY_MOD_ALT     = 1 << 2,
    KEY_MOD_SUPER   = 1 << 3,
    KEY_MOD_CAPS    = 1 << 4,
    KEY_MOD_LOCK    = 1 << 5
} ppMod;

#define PP_CALLBACKS                                 \
    X(Keyboard,     (void*, ppKey, ppMod, bool))     \
    X(MouseButton,  (void*, int, ppMod, bool))       \
    X(MouseMove,    (void*, int, int, float, float)) \
    X(MouseScroll,  (void*, float, float, ppMod))    \
    X(Resized,      (void*, int, int))               \
    X(Focus,        (void*, bool))                   \
    X(Closed,       (void*))

bool ppBegin(int w, int h, const char *title, ppFlags flags);
bool ppPoll(void);
void ppFlush(Bitmap *bitmap);
void ppEnd(void);

#define X(NAME, ARGS) \
    void(*NAME##Callback)ARGS,
void ppCallbacks(PP_CALLBACKS void *userdata);
#undef X
#define X(NAME, ARGS) \
    void pp##NAME##Callback(void(*NAME##Callback)ARGS);
PP_CALLBACKS
#undef X
void ppUserdata(void *userdata);
bool ppRunning(void);

double ppTime(void);

#endif // pp_h
