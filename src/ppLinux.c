/* ppLinux.c -- https://github.com/takeiteasy/pp
 
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

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xlocale.h>
#include <X11/XKBlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <sys/time.h>

static struct {
    Display *display;
    Window root, window;
    int screen, depth;
    int width, height;
    Bitmap resized;
    Atom delete;
    GC gc;
    XImage *img, *scaler;
    double timestamp;
    int cursorLastX, cursorLastY;
} ppLinuxInternal = {0};

struct Hints {
    unsigned long flags;
    unsigned long functions;
    unsigned long decorations;
    long input_mode;
    unsigned long status;
};

static bool ppBeginNative(int w, int h, const char *title, ppFlags flags) {
    if (!(ppLinuxInternal.display = XOpenDisplay(NULL)))
        return false;
    ppLinuxInternal.root   = DefaultRootWindow(ppLinuxInternal.display);
    ppLinuxInternal.screen = DefaultScreen(ppLinuxInternal.display);
    ppLinuxInternal.scaler = NULL;
    ppLinuxInternal.resized.buf = NULL;
    
    struct timeval tv;
    gettimeofday(&tv, NULL);
    ppLinuxInternal.timestamp = (double)tv.tv_sec + (tv.tv_usec / 1000000.0);
    
    int screen_w = DisplayWidth(ppLinuxInternal.display, ppLinuxInternal.screen);
    int screen_h = DisplayHeight(ppLinuxInternal.display, ppLinuxInternal.screen);
    
    if (flags & ppFullscreen)
        flags = ppFullscreen | ppBorderless;
    
    int x = 0, y = 0;
    if (flags & ppFullscreen || flags & ppFullscreenDesktop) {
        w = screen_w;
        h = screen_h;
    } else {
        x = screen_w / 2 - w / 2;
        y = screen_h / 2 - h / 2;
    }
    
    Visual *visual = DefaultVisual(ppLinuxInternal.display, ppLinuxInternal.screen);
    int format_c = 0;
    XPixmapFormatValues* formats = XListPixmapFormats(ppLinuxInternal.display, &format_c);
    ppLinuxInternal.depth = DefaultDepth(ppLinuxInternal.display, ppLinuxInternal.screen);
    int depth_c;
    for (int i = 0; i < format_c; ++i)
        if (ppLinuxInternal.depth == formats[i].depth) {
            depth_c = formats[i].bits_per_pixel;
            break;
        }
    XFree(formats);
    if (depth_c != 32)
        return false;
    
    XSetWindowAttributes swa;
    swa.override_redirect = True;
    swa.border_pixel = BlackPixel(ppLinuxInternal.display, ppLinuxInternal.screen);
    swa.background_pixel = BlackPixel(ppLinuxInternal.display, ppLinuxInternal.screen);
    swa.backing_store = NotUseful;
    if (!(ppLinuxInternal.window = XCreateWindow(ppLinuxInternal.display, ppLinuxInternal.root, x, y, w, h, 0, ppLinuxInternal.depth, InputOutput, visual, CWBackPixel | CWBorderPixel | CWBackingStore, &swa)))
        return false;
    ppLinuxInternal.width = w;
    ppLinuxInternal.height = h;
    
    ppLinuxInternal.delete = XInternAtom(ppLinuxInternal.display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(ppLinuxInternal.display, ppLinuxInternal.window, &ppLinuxInternal.delete, 1);
    
    XSelectInput(ppLinuxInternal.display, ppLinuxInternal.window, StructureNotifyMask | KeyPressMask | KeyReleaseMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask | ExposureMask | FocusChangeMask | EnterWindowMask | LeaveWindowMask);
    XStoreName(ppLinuxInternal.display, ppLinuxInternal.window, title);
    
    if (flags & ppFullscreen) {
        Atom p = XInternAtom(ppLinuxInternal.display, "_NET_WM_STATE_FULLSCREEN", True);
        XChangeProperty(ppLinuxInternal.display, ppLinuxInternal.window, XInternAtom(ppLinuxInternal.display, "_NET_WM_STATE", True), XA_ATOM, 32, PropModeReplace, (unsigned char*)&p, 1);
    }
    if (flags & ppBorderless) {
        struct Hints hints;
        hints.flags = 2;
        hints.decorations = 0;
        Atom p = XInternAtom(ppLinuxInternal.display, "_MOTIF_WM_HINTS", True);
        XChangeProperty(ppLinuxInternal.display, ppLinuxInternal.window, p, p, 32, PropModeReplace, (unsigned char*)&hints, 5);
    }
    
    if (flags & ppAlwaysOnTop) {
        Atom p = XInternAtom(ppLinuxInternal.display, "_NET_WM_STATE_ABOVE", False);
        XChangeProperty(ppLinuxInternal.display, ppLinuxInternal.window, XInternAtom(ppLinuxInternal.display, "_NET_WM_STATE", False), XA_ATOM, 32, PropModeReplace, (unsigned char *)&p, 1);
    }
    
    XSizeHints hints;
    hints.flags = PPosition | PMinSize | PMaxSize;
    hints.x = 0;
    hints.y = 0;
    if (flags & ppResizable) {
        hints.min_width = 0;
        hints.min_height = 0;
        hints.max_width = screen_w;
        hints.max_height = screen_h;
    } else {
        hints.min_width = w;
        hints.min_height = h;
        hints.max_width = w;
        hints.max_height = h;
    }
    XSetWMNormalHints(ppLinuxInternal.display, ppLinuxInternal.window, &hints);
    XClearWindow(ppLinuxInternal.display, ppLinuxInternal.window);
    XMapRaised(ppLinuxInternal.display, ppLinuxInternal.window);
    XFlush(ppLinuxInternal.display);
    ppLinuxInternal.gc = DefaultGC(ppLinuxInternal.display, ppLinuxInternal.screen);
    ppLinuxInternal.img = XCreateImage(ppLinuxInternal.display, CopyFromParent, ppLinuxInternal.depth, ZPixmap, 0, NULL, w, h, 32, w * 4);
    
    ppInternal.running = true;
    return true;
}

static uint8_t ConvertX11Key(KeySym sym) {
    if (sym >= 'a' && sym <= 'z')
        return (uint8_t)sym - ('a' - 'A');
    if (sym >= '0' && sym <= '9')
        return (uint8_t)sym;
    
    switch (sym) {
        case XK_KP_0:
            return KEY_PAD0;
        case XK_KP_1:
            return KEY_PAD1;
        case XK_KP_2:
            return KEY_PAD2;
        case XK_KP_3:
            return KEY_PAD3;
        case XK_KP_4:
            return KEY_PAD4;
        case XK_KP_5:
            return KEY_PAD5;
        case XK_KP_6:
            return KEY_PAD6;
        case XK_KP_7:
            return KEY_PAD7;
        case XK_KP_8:
            return KEY_PAD8;
        case XK_KP_9:
            return KEY_PAD9;
            
        case XK_KP_Multiply:
            return KEY_PADMUL;
        case XK_KP_Divide:
            return KEY_PADDIV;
        case XK_KP_Add:
            return KEY_PADADD;
        case XK_KP_Subtract:
            return KEY_PADSUB;
        case XK_KP_Decimal:
            return KEY_PADDOT;
        case XK_KP_Enter:
            return KEY_PADENTER;
            
        case XK_F1:
            return KEY_F1;
        case XK_F2:
            return KEY_F2;
        case XK_F3:
            return KEY_F3;
        case XK_F4:
            return KEY_F4;
        case XK_F5:
            return KEY_F5;
        case XK_F6:
            return KEY_F6;
        case XK_F7:
            return KEY_F7;
        case XK_F8:
            return KEY_F8;
        case XK_F9:
            return KEY_F9;
        case XK_F10:
            return KEY_F10;
        case XK_F11:
            return KEY_F11;
        case XK_F12:
            return KEY_F12;
            
        case XK_BackSpace:
            return KEY_BACKSPACE;
        case XK_Tab:
            return KEY_TAB;
        case XK_Return:
            return KEY_RETURN;
        case XK_Pause:
            return KEY_PAUSE;
        case XK_Caps_Lock:
            return KEY_CAPSLOCK;
        case XK_Escape:
            return KEY_ESCAPE;
        case XK_space:
            return KEY_SPACE;
            
        case XK_Page_Up:
            return KEY_PAGEUP;
        case XK_Page_Down:
            return KEY_PAGEDN;
        case XK_End:
            return KEY_END;
        case XK_Home:
            return KEY_HOME;
        case XK_Left:
            return KEY_LEFT;
        case XK_Up:
            return KEY_UP;
        case XK_Right:
            return KEY_RIGHT;
        case XK_Down:
            return KEY_DOWN;
        case XK_Insert:
            return KEY_INSERT;
        case XK_Delete:
            return KEY_DELETE;
            
        case XK_Meta_L:
            return KEY_LWIN;
        case XK_Meta_R:
            return KEY_RWIN;
        case XK_Num_Lock:
            return KEY_NUMLOCK;
        case XK_Scroll_Lock:
            return KEY_SCROLL;
        case XK_Shift_L:
            return KEY_LSHIFT;
        case XK_Shift_R:
            return KEY_RSHIFT;
        case XK_Control_L:
            return KEY_LCONTROL;
        case XK_Control_R:
            return KEY_RCONTROL;
        case XK_Alt_L:
            return KEY_LALT;
        case XK_Alt_R:
            return KEY_RALT;
            
        case XK_semicolon:
            return KEY_SEMICOLON;
        case XK_equal:
            return KEY_EQUALS;
        case XK_comma:
            return KEY_COMMA;
        case XK_minus:
            return KEY_MINUS;
        case XK_period:
            return KEY_DOT;
        case XK_slash:
            return KEY_SLASH;
        case XK_grave:
            return KEY_BACKTICK;
        case XK_bracketleft:
            return KEY_LSQUARE;
        case XK_backslash:
            return KEY_BACKSLASH;
        case XK_bracketright:
            return KEY_RSQUARE;
        case XK_apostrophe:
            return KEY_TICK;
    }
    return 0;
}

static int ConvertX11Mod(int state) {
    int mod_keys = 0;
    if (state & ShiftMask)
        mod_keys |= KEY_MOD_SHIFT;
    if (state & ControlMask)
        mod_keys |= KEY_MOD_CONTROL;
    if (state & Mod1Mask)
        mod_keys |= KEY_MOD_ALT;
    if (state & Mod4Mask)
        mod_keys |= KEY_MOD_SUPER;
    if (state & LockMask)
        mod_keys |= KEY_MOD_CAPS_LOCK;
    if (state & Mod2Mask)
        mod_keys |= KEY_MOD_NUM_LOCK;
    return mod_keys;
}

#define Button6 6
#define Button7 7

static int ConvertX11ModEx(int key, int state, int is_pressed) {
    int mod_keys = ConvertX11Mod(state);
    switch (key) {
        case KEY_LSHIFT:
        case KEY_RSHIFT:
            if (is_pressed)
                mod_keys |= KEY_MOD_SHIFT;
            else
                mod_keys &= ~KEY_MOD_SHIFT;
            break;
        case KEY_LCONTROL:
        case KEY_RCONTROL:
            if (is_pressed)
                mod_keys |= KEY_MOD_CONTROL;
            else
                mod_keys &= ~KEY_MOD_CONTROL;
            break;
        case KEY_LALT:
        case KEY_RALT:
            if (is_pressed)
                mod_keys |= KEY_MOD_ALT;
            else
                mod_keys &= ~KEY_MOD_ALT;
            break;
        case KEY_LWIN:
        case KEY_RWIN:
            if (is_pressed)
                mod_keys |= KEY_MOD_SUPER;
            else
                mod_keys &= ~KEY_MOD_SUPER;
            break;
    }
    return mod_keys;
}

bool ppPoll(void) {
    XEvent e;
    while (ppInternal.running && XPending(ppLinuxInternal.display)) {
        XNextEvent(ppLinuxInternal.display, &e);
        switch (e.type) {
            case KeyPress:
            case KeyRelease:
                ppCallCallback(Keyboard, ConvertX11Key(e.xkey.keycode), ConvertX11ModEx(e.xkey.keycode, e.xkey.state, e.type == KeyPress), e.type == KeyPress);
                break;
            case ButtonPress:
            case ButtonRelease:
                switch (e.xbutton.button) {
                    case Button1:
                    case Button2:
                    case Button3:
                        ppCallCallback(MouseButton, (int)e.xbutton.button, ConvertX11Mod(e.xkey.state), e.type == ButtonPress);
                        break;
                    case Button4:
                        ppCallCallback(MouseScroll, 0.f, 1.f, ConvertX11Mod(e.xkey.state));
                        break;
                    case Button5:
                        ppCallCallback(MouseScroll, 0.f, -1.f, ConvertX11Mod(e.xkey.state));
                        break;
                    case Button6:
                        ppCallCallback(MouseScroll, 1.f, 0.f, ConvertX11Mod(e.xkey.state));
                        break;
                    case Button7:
                        ppCallCallback(MouseScroll, -1.f, 0.f, ConvertX11Mod(e.xkey.state));
                        break;
                    default:
                        ppCallCallback(MouseButton, (int)(e.xbutton.button - 4), ConvertX11Mod(e.xkey.state), e.type == ButtonPress);
                        break;
                }
                break;
            case FocusIn:
            case FocusOut:
                ppCallCallback(Focus, e.type == FocusIn);
                break;
            case MotionNotify: {
                int cx = e.xmotion.x;
                int cy = e.xmotion.y;
                ppCallCallback(MouseMove, cx, cy, cx - ppLinuxInternal.cursorLastX, cy - ppLinuxInternal.cursorLastY);
                ppLinuxInternal.cursorLastX = cx;
                ppLinuxInternal.cursorLastY = cy;
                break;
            }
            case ConfigureNotify: {
                int w = e.xconfigure.width;
                int h = e.xconfigure.height;
                if (ppLinuxInternal.width == w && ppLinuxInternal.height == h)
                    break;
                ppCallCallback(Resized, w, h);
                ppLinuxInternal.width = w;
                ppLinuxInternal.height = h;
                
                if (ppLinuxInternal.scaler) {
                    ppLinuxInternal.scaler->data = NULL;
                    XDestroyImage(ppLinuxInternal.scaler);
                    ppLinuxInternal.scaler = NULL;
                }
                XClearWindow(ppLinuxInternal.display, ppLinuxInternal.window);
                break;
            }
            case ClientMessage:
                if (e.xclient.data.l[0] != ppLinuxInternal.delete)
                    break;
                ppInternal.running = false;
                break;
        }
    }
    return ppInternal.running;
}

void ppFlush(Bitmap *bitmap) {
    if (!bitmap || !bitmap->buf || !bitmap->w || !bitmap->h) {
        ppInternal.pbo = NULL;
        return;
    }
    
    if (ppLinuxInternal.width != bitmap->w || ppLinuxInternal.height != bitmap->h) {
        if (ppLinuxInternal.scaler) {
            ppLinuxInternal.scaler->data = NULL;
            XDestroyImage(ppLinuxInternal.scaler);
            ppLinuxInternal.scaler = NULL;
        }
        ppLinuxInternal.scaler = XCreateImage(ppLinuxInternal.display, CopyFromParent, ppLinuxInternal.depth, ZPixmap, 0, NULL, ppLinuxInternal.width, ppLinuxInternal.height, 32, ppLinuxInternal.width * 4);
        
        if (ppLinuxInternal.resized.buf)
            DestroyBitmap(&ppLinuxInternal.resized);
        ScaleBitmap(bitmap, ppLinuxInternal.width, ppLinuxInternal.height, &ppLinuxInternal.resized);
        
        ppLinuxInternal.scaler->data = (char*)ppLinuxInternal.resized.buf;
        XPutImage(ppLinuxInternal.display, ppLinuxInternal.window, ppLinuxInternal.gc, ppLinuxInternal.scaler, 0, 0, 0, 0, ppLinuxInternal.width, ppLinuxInternal.height);
    } else {
        ppLinuxInternal.img->data = (char*)bitmap->buf;
        XPutImage(ppLinuxInternal.display, ppLinuxInternal.window, ppLinuxInternal.gc, ppLinuxInternal.img, 0, 0, 0, 0, ppLinuxInternal.width, ppLinuxInternal.height);
    }
    XFlush(ppLinuxInternal.display);
}

void ppEnd(void) {
    if (!ppInternal.running)
        return;
    if (ppLinuxInternal.img) {
        ppLinuxInternal.img->data = NULL;
        XDestroyImage(ppLinuxInternal.img);
    }
    if (ppLinuxInternal.scaler) {
        ppLinuxInternal.scaler->data = NULL;
        XDestroyImage(ppLinuxInternal.scaler);
    }
    if (ppLinuxInternal.resized.buf)
        DestroyBitmap(&ppLinuxInternal.resized);
    if (ppLinuxInternal.window)
        XDestroyWindow(ppLinuxInternal.display, ppLinuxInternal.window);
    if (ppLinuxInternal.display)
        XCloseDisplay(ppLinuxInternal.display);
}

double ppTime(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    double now = (double)tv.tv_sec + (tv.tv_usec / 1000000.0);
    double elapsed = now - ppLinuxInternal.timestamp;
    ppLinuxInternal.timestamp = now;
    return elapsed;
}
