/* ppLive.c -- https://github.com/takeiteasy/pp

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

#define PP_IMPLEMENTATION
#include "ppLive.h"
#if defined(PP_WINDOWS)
#include <windows.h>
#include <io.h>
#define F_OK    0
#define access _access
#include "getopt_win32.h"
#ifndef _MSC_VER
#pragma comment(lib, "Psapi.lib")
#endif
#include "dlfcn_win32.h"
static FILETIME writeTime;
#else
#include <getopt.h>
#define _BSD_SOURCE // usleep()
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>
static ino_t handleID;
#endif

static void *handle = NULL;
static ppState *state = NULL;
static ppApp *app = NULL;
static ppSurface *surface = NULL;
static struct {
    int width;
    int height;
    const char *title;
    ppFlags flags;
    char *path;
} Args = {0};

static struct option long_options[] = {
    {"width", required_argument, NULL, 'w'},
    {"height", required_argument, NULL, 'h'},
    {"title", required_argument, NULL, 't'},
    {"resizable", no_argument, NULL, 'r'},
    {"top", no_argument, NULL, 'a'},
    {"usage", no_argument, NULL, 'u'},
    {"path", required_argument, NULL, 'p'},
    {NULL, 0, NULL, 0}
};

static void usage(void) {
    puts(" usage: ppLive -p [path to dylib] [options]\n");
    puts("   -w/--width     -- Window width [default: 640]");
    puts("   -h/--height    -- Window height [default: 480]");
    puts("   -t/--title     -- Window title [default: \"pp\"]");
    puts("   -r/--resizable -- Enable resizable window");
    puts("   -a/--top       -- Enable window always on top");
    puts("   -p/--path      -- Path the dynamic library [required]");
    puts("   -u/--usage     -- Display this message");
}

#if defined(PP_WINDOWS)
static FILETIME Win32GetLastWriteTime(char* path) {
    FILETIME time;
    WIN32_FILE_ATTRIBUTE_DATA data;

    if (GetFileAttributesEx(path, GetFileExInfoStandard, &data))
        time = data.ftLastWriteTime;

    return time;
}
#endif

static int ShouldReloadLibrary(void) {
#if defined(PP_WINDOWS)
    FILETIME newTime = Win32GetLastWriteTime(Args.path);
    bool result = CompareFileTime(&newTime, &writeTime);
    if (result)
        writeTime = newTime;
    return result;
#else
    struct stat attr;
    int result = !stat(Args.path, &attr) && handleID != attr.st_ino;
    if (result)
        handleID = attr.st_ino;
    return result;
#endif
}

#if defined(PP_WINDOWS)
char* RemoveExt(char* path) {
    char *ret = malloc(strlen(path) + 1);
    if (!ret)
        return NULL;
    strcpy(ret, path);
    char *ext = strrchr(ret, '.');
    if (ext)
        *ext = '\0';
    return ret;
}
#endif

static int ReloadLibrary(const char *path) {
    if (!ShouldReloadLibrary())
        return 1;

    if (handle) {
        if (app->unload)
            app->unload(state);
        dlclose(handle);
    }

#if defined(PP_WINDOWS)
    size_t newPathSize = strlen(path) + 4;
    char *newPath = malloc(sizeof(char) * newPathSize);
    char *noExt = RemoveExt(path);
    sprintf(newPath, "%s.tmp.dll", noExt);
    CopyFile(path, newPath, 0);
    handle = dlopen(newPath, RTLD_NOW);
    free(newPath);
    free(noExt);
    if (!handle)
#else
    if (!(handle = dlopen(path, RTLD_NOW)))
#endif
        goto BAIL;
    if (!(app = dlsym(handle, "pp")))
        goto BAIL;
    if (!state) {
        if (!(state = app->init()))
            goto BAIL;
    } else {
        if (app->reload)
            app->reload(state);
    }
    return 1;

BAIL:
    if (handle)
        dlclose(handle);
    handle = NULL;
#if defined(PP_WINDOWS)
    memset(&writeTime, 0, sizeof(FILETIME));
#else
    handleID = 0;
#endif
    return 0;
}

#define ppInputCallback(E) \
    if (app->event)        \
        app->event(state, &(E))

static void ppInputKeyboard(void *userdata, int key, int modifier, int isDown) {
    ppEvent e = {
        .type = ppKeyboardEvent,
        .Keyboard = {
            .key = key,
            .isdown = isDown
        },
        .modifier = modifier
    };
    ppInputCallback(e);
}

static void ppInputMouseButton(void *userdata, int button, int modifier, int isDown) {
    ppEvent e = {
        .type = ppMouseButtonEvent,
        .Mouse = {
            .button = button,
            .isdown = isDown
        },
        .modifier = modifier
    };
    ppInputCallback(e);
}

static void ppInputMouseMove(void *userdata, int x, int y, float dx, float dy) {
    ppEvent e = {
        .type = ppMouseMoveEvent,
        .Mouse = {
            .Position = {
                .x = x,
                .y = y,
                .dx = dx,
                .dy = dy
            }
        }
    };
    ppInputCallback(e);
}

static void ppInputMouseScroll(void *userdata, float dx, float dy, int modifier) {
    ppEvent e = {
        .type = ppMouseScrollEvent,
        .Mouse = {
            .Scroll = {
                .dx = dx,
                .dy = dy
            }
        }
    };
    ppInputCallback(e);
}

static void ppInputFocus(void *userdata, int isFocused) {
    ppEvent e = {
        .type = ppFocusEvent,
        .Window = {
            .focused = isFocused
        }
    };
    ppInputCallback(e);
}

static void ppInputResized(void *userdata, int w, int h) {
    ppEvent e = {
        .type = ppResizedEvent,
        .Window = {
            .Size = {
                .width = w,
                .height = h
            }
        }
    };
    ppInputCallback(e);
}

static void ppInputClosed(void *userdata) {
    ppEvent e = {
        .type = ppClosedEvent,
        .Window = {
            .closed = true
        }
    };
    ppInputCallback(e);
}

int main(int argc, char *argv[]) {
    extern char* optarg;
    extern int optopt;
    int opt;
    while ((opt = getopt_long(argc, argv, ":w:h:t:p:uar", long_options, NULL)) != -1) {
        switch (opt) {
            case 'w':
                Args.width = atoi(optarg);
                break;
            case 'h':
                Args.height = atoi(optarg);
                break;
            case 't':
                Args.title = optarg;
                break;
            case 'r':
                Args.flags |= ppResizable;
                break;
            case 'a':
                Args.flags |= ppAlwaysOnTop;
                break;
            case 'p':
                Args.path = optarg;
                break;
            case ':':
                printf("ERROR: \"-%c\" requires an value!\n", optopt);
                usage();
                return 1;
            case '?':
                printf("ERROR: Unknown argument \"-%c\"\n", optopt);
                usage();
                return 1;
            case 'u':
                usage();
                return 0;
            default:
                usage();
                return 1;
        }
    }

    if (!Args.path) {
        puts("ERROR: No path to dynamic library provided (-p/--path)");
        usage();
        return 1;
    } else {
#if !defined(PP_WINDOWS)
        if (Args.path[0] != '.' || Args.path[1] != '/') {
            char *tmp = malloc(strlen(Args.path) + 2 * sizeof(char));
            sprintf(tmp, "./%s", Args.path);
            Args.path = tmp;
        } else
            Args.path = strdup(Args.path);
#endif
    }

    if (access(Args.path, F_OK)) {
        printf("ERROR: No file found at path \"%s\"\n", Args.path);
        return 1;
    }

    if (!Args.width)
        Args.width = 640;
    if (!Args.height)
        Args.height = 480;
    ppBegin(Args.width, Args.height, Args.title ? Args.title : "pp", Args.flags);
    surface = ppNewSurface(Args.width, Args.height);

    if (!ReloadLibrary(Args.path))
        return 1;

#define X(NAME, _) ppInput##NAME,
    ppCallbacks(PP_CALLBACKS NULL);
#undef X

    while (ppPoll()) {
        if (!ReloadLibrary(Args.path))
            break;
        if (!app->tick(state, surface, ppTime()))
            break;
        ppFlush(surface->buf, surface->w, surface->h);
    }

    app->deinit(state);
    if (handle)
        dlclose(handle);
    ppFreeSurface(surface);
#if !defined(PP_WINDOWS)
    free(Args.path);
#endif
    ppEnd();
    return 0;
}
