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
    int width, height;
} ppWinInternal = {0};

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (!ppInternal.running || !ppInternal.initialized)
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
        default:
            goto DEFAULT_PROC;
    }

    return FALSE;
DEFAULT_PROC:
    return DefWindowProc(hWnd, message, wParam, lParam);
}

bool ppBegin(int w, int h, const char *title, ppFlags flags) {
    if (ppInternal.initialized)
        return false;

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
    } else if (flags & ~ppFullscreen) {
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
    memset(ppWinInternal.bmp, 0, bmpSz);
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

    ppInternal.initialized = ppInternal.running = true;
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
    if (!ppInternal.initialized)
        return;
    free(ppWinInternal.bmp);
    ReleaseDC(ppWinInternal.hwnd, ppWinInternal.hdc);
    DestroyWindow(ppWinInternal.hwnd);
}

double ppTime(void) {
    return .0;
}
