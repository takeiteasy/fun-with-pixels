/* pb_x11.c -- https://github.com/takeiteasy/fwp
 
 fun-with-pixels is a hot-reloadable software-rendering library
 
 Copyright (C) 2024  George Watson
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#define FWP_PB_IMPLEMENTATION
#include "pb.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xlocale.h>
#include <X11/XKBlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <stdlib.h>

static struct {
    Display *display;
    Window root, window;
    int screen, depth;
    int width, height;
    int *buffer;
    Atom delete;
    GC gc;
    XImage *img, *scaler;
    int cursorLastX, cursorLastY;
} pbLinuxInternal = {0};

struct Hints {
    unsigned long flags;
    unsigned long functions;
    unsigned long decorations;
    long input_mode;
    unsigned long status;
};

int pbBeginNative(int w, int h, const char *title, pbFlags flags) {
    if (!(pbLinuxInternal.display = XOpenDisplay(NULL)))
        return 0;
    pbLinuxInternal.root   = DefaultRootWindow(pbLinuxInternal.display);
    pbLinuxInternal.screen = DefaultScreen(pbLinuxInternal.display);
    pbLinuxInternal.scaler = NULL;
    pbLinuxInternal.buffer = NULL;
    
    int screen_w = DisplayWidth(pbLinuxInternal.display, pbLinuxInternal.screen);
    int screen_h = DisplayHeight(pbLinuxInternal.display, pbLinuxInternal.screen);
    
    if (flags & pbFullscreen)
        flags = pbFullscreen | pbBorderless;
    
    int x = 0, y = 0;
    if (flags & pbFullscreen || flags & pbFullscreenDesktop) {
        w = screen_w;
        h = screen_h;
    } else {
        x = screen_w / 2 - w / 2;
        y = screen_h / 2 - h / 2;
    }
    
    Visual *visual = DefaultVisual(pbLinuxInternal.display, pbLinuxInternal.screen);
    int format_c = 0;
    XPixmapFormatValues* formats = XListPixmapFormats(pbLinuxInternal.display, &format_c);
    pbLinuxInternal.depth = DefaultDepth(pbLinuxInternal.display, pbLinuxInternal.screen);
    int depth_c;
    for (int i = 0; i < format_c; ++i)
        if (pbLinuxInternal.depth == formats[i].depth) {
            depth_c = formats[i].bits_per_pixel;
            break;
        }
    XFree(formats);
    if (depth_c != 32)
        return 0;
    
    XSetWindowAttributes swa;
    swa.override_redirect = True;
    swa.border_pixel = BlackPixel(pbLinuxInternal.display, pbLinuxInternal.screen);
    swa.background_pixel = BlackPixel(pbLinuxInternal.display, pbLinuxInternal.screen);
    swa.backing_store = NotUseful;
    if (!(pbLinuxInternal.window = XCreateWindow(pbLinuxInternal.display, pbLinuxInternal.root, x, y, w, h, 0, pbLinuxInternal.depth, InputOutput, visual, CWBackPixel | CWBorderPixel | CWBackingStore, &swa)))
        return 0;
    pbInternal.windowWidth = w;
    pbInternal.windowHeight = h;
    
    pbLinuxInternal.delete = XInternAtom(pbLinuxInternal.display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(pbLinuxInternal.display, pbLinuxInternal.window, &pbLinuxInternal.delete, 1);
    
    XSelectInput(pbLinuxInternal.display, pbLinuxInternal.window, StructureNotifyMask | KeyPressMask | KeyReleaseMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask | ExposureMask | FocusChangeMask | EnterWindowMask | LeaveWindowMask);
    XStoreName(pbLinuxInternal.display, pbLinuxInternal.window, title);
    
    if (flags & pbFullscreen) {
        Atom p = XInternAtom(pbLinuxInternal.display, "_NET_WM_STATE_FULLSCREEN", True);
        XChangeProperty(pbLinuxInternal.display, pbLinuxInternal.window, XInternAtom(pbLinuxInternal.display, "_NET_WM_STATE", True), XA_ATOM, 32, PropModeReplace, (unsigned char*)&p, 1);
    }
    if (flags & pbBorderless) {
        struct Hints hints;
        hints.flags = 2;
        hints.decorations = 0;
        Atom p = XInternAtom(pbLinuxInternal.display, "_MOTIF_WM_HINTS", True);
        XChangeProperty(pbLinuxInternal.display, pbLinuxInternal.window, p, p, 32, PropModeReplace, (unsigned char*)&hints, 5);
    }
    
    if (flags & pbAlwaysOnTop) {
        Atom p = XInternAtom(pbLinuxInternal.display, "_NET_WM_STATE_ABOVE", False);
        XChangeProperty(pbLinuxInternal.display, pbLinuxInternal.window, XInternAtom(pbLinuxInternal.display, "_NET_WM_STATE", False), XA_ATOM, 32, PropModeReplace, (unsigned char *)&p, 1);
    }
    
    XSizeHints hints;
    hints.flags = PPosition | PMinSize | PMaxSize;
    hints.x = 0;
    hints.y = 0;
    if (flags & pbResizable) {
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
    XSetWMNormalHints(pbLinuxInternal.display, pbLinuxInternal.window, &hints);
    XClearWindow(pbLinuxInternal.display, pbLinuxInternal.window);
    XMapRaised(pbLinuxInternal.display, pbLinuxInternal.window);
    XFlush(pbLinuxInternal.display);
    pbLinuxInternal.gc = DefaultGC(pbLinuxInternal.display, pbLinuxInternal.screen);
    pbLinuxInternal.img = XCreateImage(pbLinuxInternal.display, CopyFromParent, pbLinuxInternal.depth, ZPixmap, 0, NULL, w, h, 32, w * 4);
    
    return 1;
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

int pbPollNative(void) {
    XEvent e;
    while (pbInternal.running && XPending(pbLinuxInternal.display)) {
        XNextEvent(pbLinuxInternal.display, &e);
        switch (e.type) {
            case KeyPress:
            case KeyRelease:
                pbCallCallback(Keyboard, ConvertX11Key(e.xkey.keycode), ConvertX11ModEx(e.xkey.keycode, e.xkey.state, e.type == KeyPress), e.type == KeyPress);
                break;
            case ButtonPress:
            case ButtonRelease:
                switch (e.xbutton.button) {
                    case Button1:
                    case Button2:
                    case Button3:
                        pbCallCallback(MouseButton, (int)e.xbutton.button, ConvertX11Mod(e.xkey.state), e.type == ButtonPress);
                        break;
                    case Button4:
                        pbCallCallback(MouseScroll, 0.f, 1.f, ConvertX11Mod(e.xkey.state));
                        break;
                    case Button5:
                        pbCallCallback(MouseScroll, 0.f, -1.f, ConvertX11Mod(e.xkey.state));
                        break;
                    case Button6:
                        pbCallCallback(MouseScroll, 1.f, 0.f, ConvertX11Mod(e.xkey.state));
                        break;
                    case Button7:
                        pbCallCallback(MouseScroll, -1.f, 0.f, ConvertX11Mod(e.xkey.state));
                        break;
                    default:
                        pbCallCallback(MouseButton, (int)(e.xbutton.button - 4), ConvertX11Mod(e.xkey.state), e.type == ButtonPress);
                        break;
                }
                break;
            case FocusIn:
            case FocusOut:
                pbCallCallback(Focus, e.type == FocusIn);
                break;
            case MotionNotify: {
                int cx = e.xmotion.x;
                int cy = e.xmotion.y;
                pbCallCallback(MouseMove, cx, cy, cx - pbLinuxInternal.cursorLastX, cy - pbLinuxInternal.cursorLastY);
                pbLinuxInternal.cursorLastX = cx;
                pbLinuxInternal.cursorLastY = cy;
                break;
            }
            case ConfigureNotify: {
                int w = e.xconfigure.width;
                int h = e.xconfigure.height;
                if (pbInternal.windowWidth == w && pbInternal.windowHeight == h)
                    break;
                pbCallCallback(Resized, w, h);
                pbInternal.windowWidth = w;
                pbInternal.windowHeight = h;
                
                if (pbLinuxInternal.scaler) {
                    pbLinuxInternal.scaler->data = NULL;
                    XDestroyImage(pbLinuxInternal.scaler);
                    pbLinuxInternal.scaler = NULL;
                }
                XClearWindow(pbLinuxInternal.display, pbLinuxInternal.window);
                break;
            }
            case ClientMessage:
                if (e.xclient.data.l[0] != pbLinuxInternal.delete)
                    break;
                pbInternal.running = 0;
                break;
        }
    }
    return pbInternal.running;
}

static int* scale(int *data, int w, int h, int nw, int nh) {
    assert(data && w && h && nw && nh);
    assert(!(w == nw && h == nh));
    int *result = malloc(sizeof(int) * nw * nh);
    int x_ratio = (int)((w << 16) / nw) + 1;
    int y_ratio = (int)((h << 16) / nh) + 1;
    int x2, y2, i, j;
    for (i = 0; i < nh; ++i) {
        int *t = result + i * nw;
        y2 = ((i * y_ratio) >> 16);
        int *p = data + y2 * w;
        int rat = 0;
        for (j = 0; j < nw; ++j) {
            x2 = (rat >> 16);
            *t++ = p[x2];
            rat += x_ratio;
        }
    }
    return result;
}

void pbFlushNative(pbImage *buffer) {
    if (!buffer || !buffer->buffer || !buffer->width || !buffer->height)
        return;
    if (pbInternal.windowWidth != buffer->width || pbInternal.windowHeight != buffer->height) {
        if (pbLinuxInternal.scaler) {
            pbLinuxInternal.scaler->data = NULL;
            XDestroyImage(pbLinuxInternal.scaler);
            pbLinuxInternal.scaler = NULL;
        }
        pbLinuxInternal.scaler = XCreateImage(pbLinuxInternal.display, CopyFromParent, pbLinuxInternal.depth, ZPixmap, 0, NULL, pbInternal.windowWidth, pbInternal.windowHeight, 32, pbInternal.windowWidth * 4);
        
        if (pbLinuxInternal.buffer)
            free(pbLinuxInternal.buffer);
        pbLinuxInternal.buffer = scale(buffer->data, buffer->width, buffer->height, pbInternal.windowWidth, pbInternal.windowHeight);
        pbLinuxInternal.scaler->data = (char*)pbLinuxInternal.buffer;
        XPutImage(pbLinuxInternal.display, pbLinuxInternal.window, pbLinuxInternal.gc, pbLinuxInternal.scaler, 0, 0, 0, 0, pbInternal.windowWidth, pbInternal.windowHeight);
    } else {
        pbLinuxInternal.img->data = (char*)buffer->buffer;
        XPutImage(pbLinuxInternal.display, pbLinuxInternal.window, pbLinuxInternal.gc, pbLinuxInternal.img, 0, 0, 0, 0, pbInternal.windowWidth, pbInternal.windowHeight);
    }
    XFlush(pbLinuxInternal.display);
}

void pbEndNative(void) {
    assert(pbInternal.running);
    if (pbLinuxInternal.scaler) {
        pbLinuxInternal.scaler->data = NULL;
        XDestroyImage(pbLinuxInternal.scaler);
    }
    if (pbLinuxInternal.buffer)
        free(pbLinuxInternal.buffer);
    pbLinuxInternal.img->data = NULL;
    XDestroyImage(pbLinuxInternal.img);
    XDestroyWindow(pbLinuxInternal.display, pbLinuxInternal.window);
    XCloseDisplay(pbLinuxInternal.display);
}

void pbSetWindowSizeNative(unsigned int w, unsigned int h) {
    pbInternal.windowWidth = w;
    pbInternal.windowHeight = h;
    XResizeWindow(pbLinuxInternal.display, pbLinuxInternal.window, w, h);
}

void pbSetWindowTitleNative(const char *title) {
    Atom nameAtom = XInternAtom(pbLinuxInternal.display, "WM_NAME", 0);
    XChangeProperty(pbLinuxInternal.display, pbLinuxInternal.window, nameAtom, XA_STRING, 8, PropModeReplace, title, strlen(title));
}
