/* ppWindows.c -- https://github.com/takeiteasy/pp
 
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

#include "ppCommon.c"
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#if defined(_MSC_VER)
#pragma comment(lib, "shell32")
#pragma comment(lib, "user32")
#pragma comment(lib, "gdi32")

#if !defined(_DLL)
extern int main(int argc, const char *argv[]);

#ifdef UNICODE
int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
#else
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#endif
{
    int n, argc;
    LPWSTR *wargv = CommandLineToArgvW(GetCommandLineW(), &argc);
    char **argv = calloc(argc + 1, sizeof(int));

    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;

    for (n = 0; n < argc; n++) {
        int len = WideCharToMultiByte(CP_UTF8, 0, wargv[n], -1, 0, 0, NULL, NULL);
        argv[n] = malloc(len);
        WideCharToMultiByte(CP_UTF8, 0, wargv[n], -1, argv[n], len, NULL, NULL);
    }
    return main(argc, argv);
}
#endif
#endif

static struct {
    WNDCLASS wnd;
    HWND hwnd;
    HDC hdc;
    BITMAPINFO *bmp;
    TRACKMOUSEEVENT tme;
    bool tmeRefresh;
    int width, height;
    int cursorLastX, cursorLastY;
    LARGE_INTEGER timestamp;
} ppWinInternal = {0};

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
    if (!ppInternal.running)
        goto DEFAULT_PROC;
    
    switch (message) {
        case WM_PAINT:
            if (!ppInternal.pbo)
                goto DEFAULT_PROC;
            ppWinInternal.bmp->bmiHeader.biWidth = ppInternal.pbo->w;
            ppWinInternal.bmp->bmiHeader.biHeight = -ppInternal.pbo->h;
            StretchDIBits(ppWinInternal.hdc, 0, 0, ppWinInternal.width, ppWinInternal.height, 0, 0, ppInternal.pbo->w, ppInternal.pbo->h, ppInternal.pbo->buf, ppWinInternal.bmp, DIB_RGB_COLORS, SRCCOPY);
            ValidateRect(hWnd, NULL);
            break;
        case WM_DESTROY:
        case WM_CLOSE:
            if (ppInternal.ClosedCallback)
                ppInternal.ClosedCallback(ppInternal.userdata);
            ppInternal.running = false;
            break;
        case WM_SIZE:
            ppWinInternal.width = LOWORD(lParam);
            ppWinInternal.height = HIWORD(lParam);
            ppCallCallback(Resized, ppWinInternal.width, ppWinInternal.height);
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
            ppCallCallback(Keyboard, ConvertWindowsKey(wParam), WindowsModState(), !((lParam >> 31) & 1));
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
            bool action = false;
            switch (message) {
                case WM_LBUTTONDOWN:
                    action = true;
                case WM_LBUTTONUP:
                    button = 1;
                    break;
                case WM_RBUTTONDOWN:
                    action = true;
                case WM_RBUTTONUP:
                    button = 2;
                    break;
                case WM_MBUTTONDOWN:
                    action = true;
                case WM_MBUTTONUP:
                    button = 3;
                    break;
                default:
                    button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? 5 : 6);
                    if (message == WM_XBUTTONDOWN)
                        action = 1;
            }
            ppCallCallback(MouseButton, button, WindowsModState(), action);
            break;
        }
        case WM_MOUSEWHEEL:
            ppCallCallback(MouseScroll, 0.f, (SHORT)HIWORD(wParam) / (float)WHEEL_DELTA, WindowsModState());
            break;
        case WM_MOUSEHWHEEL:
            ppCallCallback(MouseScroll, -((SHORT)HIWORD(wParam) / (float)WHEEL_DELTA), 0., WindowsModState());
            break;
        case WM_MOUSEMOVE: {
            if (ppWinInternal.tmeRefresh) {
                ppWinInternal.tme.cbSize = sizeof(ppWinInternal.tme);
                ppWinInternal.tme.hwndTrack = ppWinInternal.hwnd;
                ppWinInternal.tme.dwFlags = TME_HOVER | TME_LEAVE;
                ppWinInternal.tme.dwHoverTime = 1;
                TrackMouseEvent(&ppWinInternal.tme);
            }
            int cx = ((int)(short)LOWORD(lParam));
            int cy = ((int)(short)HIWORD(lParam));
            ppCallCallback(MouseMove, cx, cy, cx - ppWinInternal.cursorLastX, cy - ppWinInternal.cursorLastY);
            ppWinInternal.cursorLastX = cx;
            ppWinInternal.cursorLastY = cy;
            break;
        }
        case WM_MOUSEHOVER:
            ppWinInternal.tmeRefresh = true;
            break;
        case WM_MOUSELEAVE:
            ppWinInternal.tmeRefresh = false;
            break;
        case WM_SETFOCUS:
            ppCallCallback(Focus, true);
            break;
        case WM_KILLFOCUS:
            ppCallCallback(Focus, false);
            break;
        default:
            goto DEFAULT_PROC;
    }

    return FALSE;
DEFAULT_PROC:
    return DefWindowProc(hWnd, message, wParam, lParam);
}

static bool ppBeginNative(int w, int h, const char *title, ppFlags flags) {
    QueryPerformanceCounter(&ppWinInternal.timestamp);
    
    RECT rect = {0};
    long windowFlags = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
    if (flags & ppFullscreen) {
        flags = ppFullscreen;
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
            flags = ppFullscreenDesktop;
    }

    if (flags & ppBorderless)
        windowFlags = WS_POPUP;
    if (flags & ppResizable)
        windowFlags |= WS_MAXIMIZEBOX | WS_SIZEBOX;
    if (flags & ppFullscreenDesktop) {
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

    memset(&ppWinInternal.wnd, 0, sizeof(ppWinInternal.wnd));
    ppWinInternal.wnd.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
    ppWinInternal.wnd.lpfnWndProc = WndProc;
    ppWinInternal.wnd.hCursor = LoadCursor(0, IDC_ARROW);
    ppWinInternal.wnd.lpszClassName = title;
    if (!RegisterClass(&ppWinInternal.wnd))
        return false;
    
    ppWinInternal.width = rect.right;
    ppWinInternal.height = rect.bottom;
    if (!(ppWinInternal.hwnd = CreateWindowEx(0, title, title, windowFlags, rect.left, rect.top, rect.right, rect.bottom, 0, 0, 0, 0)))
        return false;
    if (!(ppWinInternal.hdc = GetDC(ppWinInternal.hwnd)))
        return false;

    if (flags & ppAlwaysOnTop)
        SetWindowPos(ppWinInternal.hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    ShowWindow(ppWinInternal.hwnd, SW_NORMAL);
    SetFocus(ppWinInternal.hwnd);

    size_t bmpSz = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 3;
    if (!(ppWinInternal.bmp = malloc(bmpSz)))
        return false;
    ZeroMemory(ppWinInternal.bmp, bmpSz);
    ppWinInternal.bmp->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    ppWinInternal.bmp->bmiHeader.biPlanes = 1;
    ppWinInternal.bmp->bmiHeader.biBitCount = 32;
    ppWinInternal.bmp->bmiHeader.biCompression = BI_BITFIELDS;
    ppWinInternal.bmp->bmiHeader.biWidth = w;
    ppWinInternal.bmp->bmiHeader.biHeight = -(LONG)h;
    ppWinInternal.bmp->bmiColors[0].rgbRed = 0xFF;
    ppWinInternal.bmp->bmiColors[1].rgbGreen = 0xFF;
    ppWinInternal.bmp->bmiColors[2].rgbBlue = 0xff;

    ppWinInternal.tme.cbSize = sizeof(ppWinInternal.tme);
    ppWinInternal.tme.hwndTrack = ppWinInternal.hwnd;
    ppWinInternal.tme.dwFlags = TME_HOVER | TME_LEAVE;
    ppWinInternal.tme.dwHoverTime = HOVER_DEFAULT;
    TrackMouseEvent(&ppWinInternal.tme);

    ppInternal.running = true;
    return true;
}

bool ppPoll(void) {
    static MSG msg;
    if (PeekMessage(&msg, ppWinInternal.hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return ppInternal.running;
}

void ppFlush(Bitmap *bitmap) {
    if (!bitmap || !bitmap->buf || !bitmap->w || !bitmap->h) {
        ppInternal.pbo = NULL;
        return;
    }
    ppInternal.pbo = bitmap;
    InvalidateRect(ppWinInternal.hwnd, NULL, TRUE);
    SendMessage(ppWinInternal.hwnd, WM_PAINT, 0, 0);
}

void ppEnd(void) {
    if (!ppInternal.running)
        return;
    free(ppWinInternal.bmp);
    ReleaseDC(ppWinInternal.hwnd, ppWinInternal.hdc);
    DestroyWindow(ppWinInternal.hwnd);
}

double ppTime(void) {
    LARGE_INTEGER cnt, freq;
    QueryPerformanceCounter(&cnt);
    QueryPerformanceFrequency(&freq);
    ULONGLONG diff = cnt.QuadPart - ppWinInternal.timestamp.QuadPart;
    ppWinInternal.timestamp = cnt;
    return (double)(diff / (double)freq.QuadPart);
}
