/* pp.h -- https://github.com/takeiteasy/pp
 
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
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef pp_h
#define pp_h
#if defined(__cplusplus)
extern "C" {
#endif

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

#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
#define PP_EMSCRIPTEN
#endif
#if defined(macintosh) || defined(Macintosh) || (defined(__APPLE__) && defined(__MACH__))
#define PP_MAC
#elif defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__WINDOWS__)
#define PP_WINDOWS
#elif defined(__gnu_linux__) || defined(__linux__) || defined(__unix__)
#define PP_LINUX
#endif

#if defined(PP_WINDOWS)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

/*!
 * @enum ppFlags
 * @abstract Window config flags
 * @constant ppResizable Resizable window flag
 * @constant ppFullscreen Native fullscreen window flag
 * @constant ppFullscreenDesktop Fake fullscreen window flag
 * @constant ppBorderless Remove window titlebar flag
 * @constant ppAlwaysOnTop Force window to always be on top flag
 * @discussion Flags can be combined with bitwise operators, e.g. ppResizable | ppAlwaysOnTop
 */
typedef enum {
    ppResizable         = 1 << 0,
    ppFullscreen        = 1 << 1,
    ppFullscreenDesktop = 1 << 2,
    ppBorderless        = 1 << 3,
    ppAlwaysOnTop       = 1 << 4
} ppFlags;

/*!
 * @enum ppKey
 * @abstract Keyboard scancodes
 * @discussion For letters/numbers, use ASCII ('A'-'Z' and '0'-'9')
 */
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

/*!
 * @enum ppMod
 * @abstract Keyboard modifier flags
 * @constant KEY_MOD_SHIFT Shift key
 * @constant KEY_MOD_CONTROL Control key
 * @constant KEY_MOD_ALT Alt key
 * @constant KEY_MOD_SUPER 'Super' key (Windows key)
 * @constant KEY_MOD_CAPS Caps-lock key
 * @constant KEY_MOD_LOCK Num-lock key
 * @discussion Modifier flags will be combined, so use bitwise operators for checks, e.g. flags & KEY_MOD_SHIFT
 */
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

/*!
 * @function ppBegin
 * @abstract pp initialization function
 * @param w Window width
 * @param h Window height
 * @param title Window title
 * @param flags Window flags (see: ppFlags)
 * @return Returns true if window opened correctly, false if there was an error
 * @discussion This function should be called at the start of the program
 */
EXPORT bool ppBegin(int w, int h, const char *title, ppFlags flags);
/*!
 * @function ppPoll
 * @abstract Poll window events
 * @return Returns true if window is still open, false if the window has closed
 * @discussion This function must be called every frame. Otherwise, the window will not update
 */
EXPORT bool ppPoll(void);
/*!
 * @function ppFlush
 * @abstract Draw bitmap to the window
 * @param bitmap Bitmap object to be drawn
 * @discussion This should be called every frame. Otherwise the window will be blank!
 */
EXPORT void ppFlush(Bitmap *bitmap);
/*!
 * @function ppEnd
 * @abstract Clean up pp
 * @discussion This should be called at the end of the program (or to end the program) to clean up resources
 */
EXPORT void ppEnd(void);

#define X(NAME, ARGS) \
    void(*NAME##Callback)ARGS,
/*!
 * @function ppCallbacks
 * @abstract Assign window event callbacks
 * @param KeyboardCallback Callback for keyboard events
 * @param MouseButtonCallback Callback for mouse button events
 * @param MouseMoveCallback Callback for mouse movement event
 * @param MouseScrollCallback Callback for mouse scroll event
 * @param ResizedCallback Callback for window resize event
 * @param FocusCallback Callback for window focus/blur events
 * @param ClosedCallback Callback for window close event
 * @param userdata Userdata stored by pp, passed to callback functions
 * @discussion All callbacks are optional, pass NULL to skip callback. These can also be set individually, e.g. ppKeyboardCallback(cb)
 */
EXPORT void ppCallbacks(PP_CALLBACKS void *userdata);
#undef X
#define X(NAME, ARGS) \
    EXPORT void pp##NAME##Callback(void(*NAME##Callback)ARGS);
PP_CALLBACKS
#undef X
/*!
 * @function ppUserdata
 * @abstract Set custom userdata to be passed to callback functions
 * @param Userdata stored by pp, passed to callback functions
 */
EXPORT void ppUserdata(void *userdata);
/*!
 * @function ppRunning
 * @abstract Check if the window is still open
 * @return Returns true is window is still open, false if it's not!
 */
EXPORT bool ppRunning(void);
/*!
 * @function ppTime
 * @abstract Returns the elapsed time in millisecons since the program started
 * @return Elapsed program time in milliseconds
 */
EXPORT double ppTime(void);

#if defined(__cplusplus)
}
#endif
#endif // pp_h
