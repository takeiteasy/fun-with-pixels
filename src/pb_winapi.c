/* pb_winapi.c -- https://github.com/takeiteasy/fwp
 
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

#define PP_IMPLEMENTATION
#include "pb.h"
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#if defined(_MSC_VER)
#pragma comment(lib, "user32")
#pragma comment(lib, "gdi32")

static struct {
    WNDCLASS wnd;
    HWND hwnd;
    HDC hdc;
    BITMAPINFO *bmp;
    TRACKMOUSEEVENT tme;
    int tmeRefresh;
    int width, height;
    int cursorLastX, cursorLastY;
} pbWinInternal = {0};

static int WindowsModState(void) {
    int mods = 0;
    
    if (GetKeyState(VK_SHIFT) & 0x8000)
        mods |= KEY_MOD_SHIFT;
    if (GetKeyState(VK_CONTROL) & 0x8000)
        mods |= KEY_MOD_CONTROL;
    if (GetKeyState(VK_MENU) & 0x8000)
        mods |= KEY_MOD_ALT;
    if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000)
        mods |= KEY_MOD_SUPER;
    if (GetKeyState(VK_CAPITAL) & 1)
        mods |= KEY_MOD_CAPS_LOCK;
    if (GetKeyState(VK_NUMLOCK) & 1)
        mods |= KEY_MOD_NUM_LOCK;
    
    return mods;
}

static int ConvertWindowsKey(int key) {
    if (key >= 'A' && key <= 'Z')
        return key;
    if (key >= '0' && key <= '9')
        return key;
    switch (key) {
        case VK_BACK:
            return KEY_BACKSPACE;
        case VK_TAB:
            return KEY_TAB;
        case VK_RETURN:
            return KEY_RETURN;
        case VK_SHIFT:
            return KEY_SHIFT;
        case VK_CONTROL:
            return KEY_CONTROL;
        case VK_MENU:
            return KEY_ALT;
        case VK_PAUSE:
            return KEY_PAUSE;
        case VK_CAPITAL:
            return KEY_CAPSLOCK;
        case VK_ESCAPE:
            return KEY_ESCAPE;
        case VK_SPACE:
            return KEY_SPACE;
        case VK_PRIOR:
            return KEY_PAGEUP;
        case VK_NEXT:
            return KEY_PAGEDN;
        case VK_END:
            return KEY_END;
        case VK_HOME:
            return KEY_HOME;
        case VK_LEFT:
            return KEY_LEFT;
        case VK_UP:
            return KEY_UP;
        case VK_RIGHT:
            return KEY_RIGHT;
        case VK_DOWN:
            return KEY_DOWN;
        case VK_INSERT:
            return KEY_INSERT;
        case VK_DELETE:
            return KEY_DELETE;
        case VK_LWIN:
            return KEY_LWIN;
        case VK_RWIN:
            return KEY_RWIN;
        case VK_NUMPAD0:
            return KEY_PAD0;
        case VK_NUMPAD1:
            return KEY_PAD1;
        case VK_NUMPAD2:
            return KEY_PAD2;
        case VK_NUMPAD3:
            return KEY_PAD3;
        case VK_NUMPAD4:
            return KEY_PAD4;
        case VK_NUMPAD5:
            return KEY_PAD5;
        case VK_NUMPAD6:
            return KEY_PAD6;
        case VK_NUMPAD7:
            return KEY_PAD7;
        case VK_NUMPAD8:
            return KEY_PAD8;
        case VK_NUMPAD9:
            return KEY_PAD9;
        case VK_MULTIPLY:
            return KEY_PADMUL;
        case VK_ADD:
            return KEY_PADADD;
        case VK_SEPARATOR:
            return KEY_PADENTER;
        case VK_SUBTRACT:
            return KEY_PADSUB;
        case VK_DECIMAL:
            return KEY_PADDOT;
        case VK_DIVIDE:
            return KEY_PADDIV;
        case VK_F1:
            return KEY_F1;
        case VK_F2:
            return KEY_F2;
        case VK_F3:
            return KEY_F3;
        case VK_F4:
            return KEY_F4;
        case VK_F5:
            return KEY_F5;
        case VK_F6:
            return KEY_F6;
        case VK_F7:
            return KEY_F7;
        case VK_F8:
            return KEY_F8;
        case VK_F9:
            return KEY_F9;
        case VK_F10:
            return KEY_F10;
        case VK_F11:
            return KEY_F11;
        case VK_F12:
            return KEY_F12;
        case VK_NUMLOCK:
            return KEY_NUMLOCK;
        case VK_SCROLL:
            return KEY_SCROLL;
        case VK_LSHIFT:
            return KEY_LSHIFT;
        case VK_RSHIFT:
            return KEY_RSHIFT;
        case VK_LCONTROL:
            return KEY_LCONTROL;
        case VK_RCONTROL:
            return KEY_RCONTROL;
        case VK_LMENU:
            return KEY_LALT;
        case VK_RMENU:
            return KEY_RALT;
        case VK_OEM_1:
            return KEY_SEMICOLON;
        case VK_OEM_PLUS:
            return KEY_EQUALS;
        case VK_OEM_COMMA:
            return KEY_COMMA;
        case VK_OEM_MINUS:
            return KEY_MINUS;
        case VK_OEM_PERIOD:
            return KEY_DOT;
        case VK_OEM_2:
            return KEY_SLASH;
        case VK_OEM_3:
            return KEY_BACKTICK;
        case VK_OEM_4:
            return KEY_LSQUARE;
        case VK_OEM_5:
            return KEY_BACKSLASH;
        case VK_OEM_6:
            return KEY_RSQUARE;
        case VK_OEM_7:
            return KEY_TICK;
    }
    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (!pbInternal.running)
        goto DEFAULT_PROC;
    
    switch (message) {
        case WM_PAINT:
            if (!pbInternal.pbo)
                goto DEFAULT_PROC;
            pbWinInternal.bmp->bmiHeader.biWidth = pbInternal.w;
            pbWinInternal.bmp->bmiHeader.biHeight = -pbInternal.h;
            StretchDIBits(pbWinInternal.hdc, 0, 0, pbInternal.windowWidth, pbInternal.windowHeight, 0, 0, pbInternal.w, pbInternal.h, pbInternal.data, pbWinInternal.bmp, DIB_RGB_COLORS, SRCCOPY);
            ValidateRect(hWnd, NULL);
            break;
        case WM_DESTROY:
        case WM_CLOSE:
            if (pbInternal.ClosedCallback)
                pbInternal.ClosedCallback(pbInternal.userdata);
            pbInternal.running = 0;
            break;
        case WM_SIZE:
            pbInternal.windowWidth = LOWORD(lParam);
            pbInternal.windowHeight = HIWORD(lParam);
            pbCallCallback(Resized, pbInternal.windowWidth, pbInternal.windowHeight);
            break;
        case WM_MENUCHAR:
            // Disable beep on Alt+Enter
            if (LOWORD(wParam) == VK_RETURN)
                return MNC_CLOSE << 16;
            return DefWindowProcW(hWnd, message, wParam, lParam);
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
            pbCallCallback(Keyboard, ConvertWindowsKey(wParam), WindowsModState(), !((lParam >> 31) & 1));
            break;
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        case WM_XBUTTONUP:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONDBLCLK:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONDBLCLK: {
            int button = 0;
            int action = 0;
            switch (message) {
                case WM_LBUTTONDOWN:
                    action = 1;
                case WM_LBUTTONUP:
                    button = 1;
                    break;
                case WM_RBUTTONDOWN:
                    action = 1;
                case WM_RBUTTONUP:
                    button = 2;
                    break;
                case WM_MBUTTONDOWN:
                    action = 1;
                case WM_MBUTTONUP:
                    button = 3;
                    break;
                default:
                    button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? 5 : 6);
                    if (message == WM_XBUTTONDOWN)
                        action = 1;
            }
            pbCallCallback(MouseButton, button, WindowsModState(), action);
            break;
        }
        case WM_MOUSEWHEEL:
            pbCallCallback(MouseScroll, 0.f, (SHORT)HIWORD(wParam) / (float)WHEEL_DELTA, WindowsModState());
            break;
        case WM_MOUSEHWHEEL:
            pbCallCallback(MouseScroll, -((SHORT)HIWORD(wParam) / (float)WHEEL_DELTA), 0., WindowsModState());
            break;
        case WM_MOUSEMOVE: {
            if (pbWinInternal.tmeRefresh) {
                pbWinInternal.tme.cbSize = sizeof(pbWinInternal.tme);
                pbWinInternal.tme.hwndTrack = pbWinInternal.hwnd;
                pbWinInternal.tme.dwFlags = TME_HOVER | TME_LEAVE;
                pbWinInternal.tme.dwHoverTime = 1;
                TrackMouseEvent(&pbWinInternal.tme);
            }
            int cx = ((int)(short)LOWORD(lParam));
            int cy = ((int)(short)HIWORD(lParam));
            pbCallCallback(MouseMove, cx, cy, cx - pbWinInternal.cursorLastX, cy - pbWinInternal.cursorLastY);
            pbWinInternal.cursorLastX = cx;
            pbWinInternal.cursorLastY = cy;
            break;
        }
        case WM_MOUSEHOVER:
            pbWinInternal.tmeRefresh = 1;
            break;
        case WM_MOUSELEAVE:
            pbWinInternal.tmeRefresh = 0;
            break;
        case WM_SETFOCUS:
            pbCallCallback(Focus, 1);
            break;
        case WM_KILLFOCUS:
            pbCallCallback(Focus, 0);
            break;
        default:
            goto DEFAULT_PROC;
    }

    return 0;
DEFAULT_PROC:
    return DefWindowProc(hWnd, message, wParam, lParam);
}

int pbBeginNative(int w, int h, const char *title, pbFlags flags) {
    RECT rect = {0};
    long windowFlags = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
    if (flags & pbFullscreen) {
        flags = pbFullscreen;
        rect.right = GetSystemMetrics(SM_CXSCREEN);
        rect.bottom = GetSystemMetrics(SM_CYSCREEN);
        windowFlags = WS_POPUP & ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);

        DEVMODE settings = {0};
        EnumDisplaySettings(0, 0, &settings);
        settings.dmPelsWidth = GetSystemMetrics(SM_CXSCREEN);
        settings.dmPelsHeight = GetSystemMetrics(SM_CYSCREEN);
        settings.dmBitsPerPel = 32;
        settings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        if (ChangeDisplaySettings(&settings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
            flags = pbFullscreenDesktop;
    }

    if (flags & pbBorderless)
        windowFlags = WS_POPUP;
    if (flags & pbResizable)
        windowFlags |= WS_MAXIMIZEBOX | WS_SIZEBOX;
    if (flags & pbFullscreenDesktop) {
        windowFlags = WS_OVERLAPPEDWINDOW;

        int width = GetSystemMetrics(SM_CXFULLSCREEN);
        int height = GetSystemMetrics(SM_CYFULLSCREEN);

        rect.right = width;
        rect.bottom = height;
        AdjustWindowRect(&rect, windowFlags, 0);
        if (rect.left < 0) {
            width += rect.left * 2;
            rect.right += rect.left;
            rect.left = 0;
        }
        if (rect.bottom > (LONG)height) {
            height -= (rect.bottom - height);
            rect.bottom += (rect.bottom - height);
            rect.top = 0;
        }
    } else {
        rect.right = w;
        rect.bottom = h;

        AdjustWindowRect(&rect, windowFlags, 0);

        rect.right -= rect.left;
        rect.bottom -= rect.top;

        rect.left = (GetSystemMetrics(SM_CXSCREEN) - rect.right) / 2;
        rect.top = (GetSystemMetrics(SM_CYSCREEN) - rect.bottom + rect.top) / 2;
    }

    memset(&pbWinInternal.wnd, 0, sizeof(pbWinInternal.wnd));
    pbWinInternal.wnd.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
    pbWinInternal.wnd.lpfnWndProc = WndProc;
    pbWinInternal.wnd.hCursor = LoadCursor(0, IDC_ARROW);
    pbWinInternal.wnd.lpszClassName = title;
    if (!RegisterClass(&pbWinInternal.wnd))
        return 0;
    
    pbInternal.windowWidth = rect.right;
    pbInternal.windowHeight = rect.bottom;
    if (!(pbWinInternal.hwnd = CreateWindowEx(0, title, title, windowFlags, rect.left, rect.top, rect.right, rect.bottom, 0, 0, 0, 0)))
        return 0;
    if (!(pbWinInternal.hdc = GetDC(pbWinInternal.hwnd)))
        return 0;

    if (flags & pbAlwaysOnTop)
        SetWindowPos(pbWinInternal.hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    ShowWindow(pbWinInternal.hwnd, SW_NORMAL);
    SetFocus(pbWinInternal.hwnd);

    size_t bmpSz = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 3;
    if (!(pbWinInternal.bmp = malloc(bmpSz)))
        return 0;
    ZeroMemory(pbWinInternal.bmp, bmpSz);
    pbWinInternal.bmp->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pbWinInternal.bmp->bmiHeader.biPlanes = 1;
    pbWinInternal.bmp->bmiHeader.biBitCount = 32;
    pbWinInternal.bmp->bmiHeader.biCompression = BI_BITFIELDS;
    pbWinInternal.bmp->bmiHeader.biWidth = w;
    pbWinInternal.bmp->bmiHeader.biHeight = -(LONG)h;
    pbWinInternal.bmp->bmiColors[0].rgbRed = 0xFF;
    pbWinInternal.bmp->bmiColors[1].rgbGreen = 0xFF;
    pbWinInternal.bmp->bmiColors[2].rgbBlue = 0xff;

    pbWinInternal.tme.cbSize = sizeof(pbWinInternal.tme);
    pbWinInternal.tme.hwndTrack = pbWinInternal.hwnd;
    pbWinInternal.tme.dwFlags = TME_HOVER | TME_LEAVE;
    pbWinInternal.tme.dwHoverTime = HOVER_DEFAULT;
    TrackMouseEvent(&pbWinInternal.tme);

    return 1;
}

int pbPollNative(void) {
    static MSG msg;
    if (PeekMessage(&msg, pbWinInternal.hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return pbInternal.running;
}

void pbFlushNative(pbImage *buffer) {
    if (!buffer || !buffer->buffer || !buffer->width || !buffer->height)
        return;
    pbInternal.data = buffer->buffer;
    pbInternal.w = buffer->width;
    pbInternal.h = buffer->height;
    InvalidateRect(pbWinInternal.hwnd, NULL, 1);
    SendMessage(pbWinInternal.hwnd, WM_PAINT, 0, 0);
}

void pbEndNative(void) {
    assert(pbInternal.running);
    free(pbWinInternal.bmp);
    ReleaseDC(pbWinInternal.hwnd, pbWinInternal.hdc);
    DestroyWindow(pbWinInternal.hwnd);
}

void pbSetWindowSizeNative(unsigned int w, unsigned int h) {
    pbInternal.windowWidth = w;
    pbInternal.windowHeight = h;
    SetWindowPos(pbWinInternal.hwnd, HWND_TOP, 0, 0, w, h, SWP_SHOWWINDOW | SWP_FRAMECHANGED);
}

void pbSetWindowTitleNative(const char *title) {
    SetWindowText(state.hwnd, title);
}
