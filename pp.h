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

#if !defined(PP_LIVE_LIBRARY)
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
#endif

EXPORT unsigned int ppRandomBits(void);
EXPORT float ppRandomFloat(void);
EXPORT double ppRandomDouble(void);
EXPORT unsigned int ppRandomInt(int max);
EXPORT float ppRandomFloatRange(float min, float max);
EXPORT double ppRandomDoubleRange(double min, double max);
EXPORT unsigned int ppRandomIntRange(int min, int max);

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
 * @enum Color
 * @abstract A list of colors with names
 */
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
} Color;

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
 * @function FillBitmap
 * @abstract Fill a bitmap with a given color
 * @param b Bitmap object
 * @param col Color
 */
EXPORT void FillBitmap(Bitmap* b, int col);
/*!
 * @function FloodBitmap
 * @abstract Flood fill on bitmap object
 * @param b Bitmap object
 * @param x X start position
 * @param y Y start position
 * @param col Color
 */
EXPORT void FloodBitmap(Bitmap* b, int x, int y, int col);
/*!
 * @function ClearBitmap
 * @abstract Clear a bitmap
 * @param b Bitmap object
 * @discussion Basically the same as calling FillBitmap(&bitmap, Black)
 */
EXPORT void ClearBitmap(Bitmap *b);
/*!
 * @function BSet
 * @abstract Set and blend colors
 * @param b Bitmap object
 * @param x X position
 * @param y Y position
 * @param col Color to set
 */
EXPORT void BSet(Bitmap *b, int x, int y, int col);
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
/*!
 * @function PasteBitmap
 * @abstract Blit one bitmap onto another at point
 * @param dst Bitmap to blit to
 * @param src Bitmap to blit
 * @param x X position
 * @param y Y position
 * @return Boolean of success/failure
 */
EXPORT bool PasteBitmap(Bitmap *dst, Bitmap *src, int x, int y);
/*!
 * @function PasteBitmapClip
 * @abstract Blit one bitmap onto another at point with clipping rect
 * @param dst Bitmap to blit to
 * @param src Bitmap to blit
 * @param x X position
 * @param y Y position
 * @param rx Clip rect X
 * @param ry Clip rect Y
 * @param rw Clip rect width
 * @param rh Clip rect height
 * @return Boolean of success/failure
 */
EXPORT bool PasteBitmapClip(Bitmap *dst, Bitmap *src, int x, int y, int rx, int ry, int rw, int rh);
/*!
 * @function CopyBitmap
 * @abstract Create a copy of a bitmap
 * @param a Original bitmap object
 * @param b New bitmap object to be allocated
 * @return Boolean of success/failure
 */
EXPORT bool CopyBitmap(Bitmap *a, Bitmap *b);
/*!
 * @function PassthruBitmap
 * @abstract Loop through each pixel of bitmap and run position and color through a callback.
 * @param b Bitmap object
 * @param fn Callback function
 * @discussion Return value of the callback is the new color at the position
 */
EXPORT void PassthruBitmap(Bitmap *b, int(*fn)(int x, int y, int col));
/*!
 * @function ScaleBitmap
 * @abstract Scale bitmap to given size
 * @param a Original bitmap object
 * @param nw New width
 * @param nh New height
 * @param b New bitmap object to be allocated
 * @return Boolean of success/failure
 */
EXPORT bool ScaleBitmap(Bitmap *a, int nw, int nh, Bitmap *b);
/*!
 * @function RotateBitmap
 * @abstract Rotate a bitmap by a given degree
 * @param a Original bitmap object
 * @param angle Angle to rotate by
 * @param b New bitmap object to be allocated
 * @return Boolean of success/failure
 */
EXPORT bool RotateBitmap(Bitmap *a, float angle, Bitmap *b);
/*!
 * @function DrawLine
 * @abstract Simple Bresenham line
 * @param b Bitmap object
 * @param x0 Vector A X position
 * @param y0 Vector A Y position
 * @param x1 Vector B X position
 * @param y1 Vector B Y position
 * @param col Color of line
 */
EXPORT void DrawLine(Bitmap *b, int x0, int y0, int x1, int y1, int col);
/*!
 * @function DrawCricle
 * @abstract Draw a circle
 * @param b Bitmap object
 * @param xc Centre X position
 * @param yc Centre Y position
 * @param r Circle radius
 * @param col Color of cricle
 * @param fill Fill circle boolean
 */
EXPORT void DrawCircle(Bitmap *b, int xc, int yc, int r, int col, bool fill);
/*!
 * @function DrawRect
 * @abstract Draw a rectangle
 * @param b Bitmap object
 * @param x X position
 * @param y Y position
 * @param w Rectangle width
 * @param h Rectangle height
 * @param col Color of rectangle
 * @param fill Fill rectangle boolean
 */
EXPORT void DrawRect(Bitmap *b, int x, int y, int w, int h, int col, bool fill);
/*!
 * @function DrawTri
 * @abstract Draw a triangle
 * @param b Bitmap object
 * @param x0 Vector A X position
 * @param y0 Vector A Y position
 * @param x1 Vector B X position
 * @param y1 Vector B Y position
 * @param x2 Vector C X position
 * @param y2 Vector C Y position
 * @param col Color of line
 * @param fill Fill triangle boolean
 */
EXPORT void DrawTri(Bitmap *b, int x0, int y0, int x1, int y1, int x2, int y2, int col, bool fill);

/*!
 * @function DrawCharacter
 * @abstract Render a character to a bitmap
 * @param b Bitmap destination
 * @param c Character to render
 * @param x X offset
 * @param y Y offset
 * @param col Color of character
 */
EXPORT void DrawCharacter(Bitmap *b, char c, int x, int y, int col);
/*!
 * @function DrawString
 * @abstract Render a string to a bitmap
 * @param b Bitmap destination
 * @param str String to render
 * @param x X offset
 * @param y Y offset
 * @param col Color of character
 */
EXPORT void DrawString(Bitmap *b, const char *str, int x, int y, int col);
/*!
 * @function DrawStringFormat
 * @abstract Render a formatted string to a bitmap
 * @param b Bitmap destination
 * @param x X offset
 * @param y Y offset
 * @param col Color of character
 * @param fmt Formatting string
 * @param ... Formatting paramters
 */
EXPORT void DrawStringFormat(Bitmap *b, int x, int y, int col, const char *fmt, ...);

/*!
 * @function LoadBitmap
 * @abstract Load a .png file to bitmap object
 * @param out Bitmap object to load data to
 * @param path Path to .png file
 * @return Boolean of success/failure
 */
EXPORT bool LoadBitmap(Bitmap *out, const char *path);
/*!
 * @function LoadBitmapMemory
 * @abstract Load a .png from memory to bitmap object
 * @param out Bitmap object to load data to
 * @param data .png data
 * @param length Length of data
 * @return Boolean of success/failure
 */
EXPORT bool LoadBitmapMemory(Bitmap *out, const void *data, size_t length);
/*!
 * @function SaveBitmap
 * @abstract Save bitmap to .png file
 * @param b Bitmap object to write to disk
 * @param path Path to save bitmap
 * @return Boolean of success/failure
 */
EXPORT bool SaveBitmap(Bitmap *b, const char *path);
/*!
 * @function FlipBitmapHorizontal
 * @abstract Flip a bitmap horizontally
 * @param a Source bitmap
 * @param b Destination bitmap
 * @return Boolean of success/failure
 */
EXPORT bool FlipBitmapHorizontal(Bitmap *a, Bitmap *b);
/*!
 * @function FlipBitmapVertical
 * @abstract Flip a bitmap vertically
 * @param a Source bitmap
 * @param b Destination bitmap
 * @return Boolean of success/failure
 */
EXPORT bool FlipBitmapVertical(Bitmap *a, Bitmap *b);
/*!
 * @function GenBitmapCheckerboard
 * @abstract Generate a checkboard image
 * @param b Bitmap output
 * @param w Bitmap width
 * @param h Bitmap height
 * @param cw Checkerboard width
 * @param ch Checkboard height
 * @param col1 First checkerboard color
 * @param col2 Second checkboard color
 * @return Boolean of success/failure
 */
EXPORT bool GenBitmapCheckerboard(Bitmap *b, int w, int h, int cw, int ch, int col1, int col2);
/*!
 * @function GenBitmapGradientHorizontal
 * @abstract Generate a horizontal gradient bitmap
 * @param b Bitmap output
 * @param w Bitmap width
 * @param h Bitmap height
 * @param col1 First gradient color
 * @param col2 Second gradient color
 * @return Boolean of success/failure
 */
EXPORT bool GenBitmapGradientHorizontal(Bitmap *b, int w, int h, int col1, int col2);
/*!
 * @function GenBitmapGradientVertical
 * @abstract Generate a vertical gradient bitmap
 * @param b Bitmap output
 * @param w Bitmap width
 * @param h Bitmap height
 * @param col1 First gradient color
 * @param col2 Second gradient color
 * @return Boolean of success/failure
 */
EXPORT bool GenBitmapGradientVertical(Bitmap *b, int w, int h, int col1, int col2);
/*!
 * @function GenBitmapGradientRadial
 * @abstract Create a radial gradient bitmap
 * @param b Bitmap output
 * @param w Bitmap width
 * @param h Bitmap height
 * @param d Radial density
 * @param col1 First gradient color (inner)
 * @param col2 Second gradient color (outer)
 * @return Boolean of success/failure
 */
EXPORT bool GenBitmapGradientRadial(Bitmap *b, int w, int h, float d, int col1, int col2);
/*!
 * @function GenBitmapWhiteNoise
 * @abstract Create a white noise bitmap
 * @param b Bitmap output
 * @param w Bitmap width
 * @param h Bitmap height
 * @param factor Noise factor
 * @return Boolean of success/failure
 */
EXPORT bool GenBitmapWhiteNoise(Bitmap *b, int w, int h, float factor);
/*!
 * @function GenBitmapFBMNoise
 * @abstract Create a simplex or perlin noise bitmap with FBM
 * @param b Bitmap output
 * @param w Bitmap width
 * @param h Bitmap height
 * @param offsetX Noise offset
 * @param offsetY Noise offset
 * @param scale Noise scale
 * @param lacunarity Controls the size of the secondary fine details (2.0 is a good start)
 * @param gain The amount to multiply the noise's amplitude by (0.5 is a good start)
 * @param octaves Describes the level of detail of the noise (8 is a good start)
 * @return Boolean of success/failure
 * @discussion Define PP_SIMPLEX_NOISE or PP_PERLIN_NOISE to select noise algorithm (PP_SIMPLEX_NOISE is default)
 */
EXPORT bool GenBitmapFBMNoise(Bitmap *b, int w, int h, int offsetX, int offsetY, float scale, float lacunarity, float gain, int octaves);

#if defined(PP_LIVE) || defined(PP_LIVE_LIBRARY)
typedef struct ppState ppState;

typedef enum {
#define X(NAME, ARGS) NAME##Event,
    PP_CALLBACKS
#undef X
} ppEventType;

typedef struct ppEvent {
    struct {
        int button;
        bool isdown;
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
        bool isdown;
    } Keyboard;
    ppMod modifier;
    struct {
        bool focused, closed;
        struct {
            int width, height;
        } Size;
    } Window;
    ppEventType type;
    struct ppEvent *next;
} ppEvent;

typedef struct {
    ppState*(*init)(void);
    void(*deinit)(ppState*);
    void(*reload)(ppState*);
    void(*unload)(ppState*);
    bool(*event)(ppState*, ppEvent*);
    bool(*tick)(ppState*, Bitmap*, double);
} ppApp;

#ifndef _MSC_VER
extern const ppApp pp;
#endif
#endif

#if defined(__cplusplus)
}
#endif
#endif // pp_h
