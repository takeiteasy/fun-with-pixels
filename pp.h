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
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#ifndef pp_h
#define pp_h
#if defined(__cplusplus)
extern "C" {
#endif

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
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <assert.h>
#include <time.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <float.h>
#include <setjmp.h>
#include <errno.h>

#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
#include <emscripten.h>
#define PP_EMSCRIPTEN
#elif defined(macintosh) || defined(Macintosh) || (defined(__APPLE__) && defined(__MACH__))
#define PP_MAC
#elif defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__WINDOWS__)
#define PP_WINDOWS
#elif defined(__gnu_linux__) || defined(__linux__) || defined(__unix__)
#define PP_LINUX
#endif

#if defined(PP_WINDOWS) && !defined(PP_NO_EXPORT)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

/*!
 * @struct Bitmap
 * @abstract Bitmap object to hold image data
 * @field buf Pixel data
 * @field w Width of image
 * @field h Height of image
 */
typedef struct {
    int *buf, w, h;
} Bitmap;

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
 * @constant KEY_MOD_CAPS_LOCK Caps-lock key
 * @constant KEY_MOD_NUM_LOCK Num-lock key
 * @discussion Modifier flags will be combined, so use bitwise operators for checks, e.g. flags & KEY_MOD_SHIFT
 */
typedef enum {
    KEY_MOD_SHIFT     = 1 << 0,
    KEY_MOD_CONTROL   = 1 << 1,
    KEY_MOD_ALT       = 1 << 2,
    KEY_MOD_SUPER     = 1 << 3,
    KEY_MOD_CAPS_LOCK = 1 << 4,
    KEY_MOD_NUM_LOCK  = 1 << 5
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
 * @param userdata stored by pp, passed to callback functions
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

/*!
 * @function RGBA
 * @abstract Create a packed RGBA integer
 * @param r R channel
 * @param g G channel
 * @param b B channel
 * @param a A channel
 * @return Packed RGBA integer
 */
EXPORT int RGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
/*!
 * @function RGB
 * @abstract Convert RGB to packed integer
 * @param r R channel
 * @param g G channel
 * @param b B channel
 * @return Packed RGB integer
 * @discussion Alpha is 255 by default
 */
EXPORT int RGB(unsigned char r, unsigned char g, unsigned char b);
/*!
 * @function RGBA1
 * @abstract Create a packed RGBA integer from one channel
 * @param c R,G + B channels
 * @param a A channel
 * @return Packed RGBA integer
 * @discussion This is a convenience wrapper for RGBA, e.g. RGBA(100, 100, 100, 255) == RGBA1(100, 255)
 */
EXPORT int RGBA1(unsigned char c, unsigned char a);
/*!
 * @function RGB1
 * @abstract Create a packed RGBA integer from one channel
 * @param c R, G + B channels
 * @return Packed RGBA integer
 * @discussion This is a convenience wrapper for RGB, e.g. RGB(100, 100, 100) == RGB1(100)
 */
EXPORT int RGB1(unsigned char c);
/*!
 * @function Rgba
 * @abstract Retrieve R channel from packed RGBA integer
 * @param c Packed RGBA integer
 * @return R channel value
 */
EXPORT unsigned char Rgba(int c);
/*!
 * @function rGba
 * @abstract Retrieve G channel from packed RGBA integer
 * @param c Packed RGBA integer
 * @return G channel value
 */
EXPORT unsigned char rGba(int c);
/*!
 * @function rgBa
 * @abstract Retrieve B channel from packed RGBA integer
 * @param c Packed RGBA integer
 * @return B channel value
 */
EXPORT unsigned char rgBa(int c);
/*!
 * @function rgbA
 * @abstract Retrieve A channel from packed RGBA integer
 * @param c Packed RGBA integer
 * @return A channel value
 */
EXPORT unsigned char rgbA(int c);
/*!
 * @function rGBA
 * @abstract Modify R channel of packed RGBA integer
 * @param c Packed RGBA integer
 * @param r New R channel value
 * @return Modified packed RGBA integer
 */
EXPORT int rGBA(int c, unsigned char r);
/*!
 * @function RgBA
 * @abstract Modify G channel of packed RGBA integer
 * @param c Packed RGBA integer
 * @param g New G channel
 * @return Modified packed RGBA integer
 */
EXPORT int RgBA(int c, unsigned char g);
/*!
 * @function RGbA
 * @abstract Modify B channel of packed RGBA integer
 * @param c Packed RGBA integer
 * @param b New B channel
 * @return Modified packed RGBA integer
 */
EXPORT int RGbA(int c, unsigned char b);
/*!
 * @function RGBa
 * @abstract Modify A channel of packed RGBA integer
 * @param c Packed RGBA integer
 * @param a New A channel
 * @return Modified packed RGBA integer
 */
EXPORT int RGBa(int c, unsigned char a);

/*!
 * @function InitBitmap
 * @abstract Create a new bitmap object
 * @param b Pointer to bitmap object to create
 * @param w Width of new bitmap
 * @param h Height of new bitmap
 * @return Boolean for success/failure
 */
EXPORT bool InitBitmap(Bitmap* b, unsigned int w, unsigned int h);
/*!
 * @function DestroyBitmap
 * @abstract Free allocated image data
 * @param b Reference to bitmap object
 */
EXPORT void DestroyBitmap(Bitmap* b);

/*!
 * @function PSet
 * @abstract Set bitmap pixel color (without blending)
 * @param b Bitmap object
 * @param x X position
 * @param y Y position
 * @param col Color to set
 */
EXPORT void PSet(Bitmap *b, int x, int y, int col);
/*!
 * @function PGet
 * @abstract Get bitmap pixel color at position
 * @param b Bitmap object
 * @param x X position
 * @param y Y position
 * @return Pixel color
 */
EXPORT int PGet(Bitmap *b, int x, int y);

#if defined(__cplusplus)
}
#endif
#endif // pp_h
