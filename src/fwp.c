/* fwp.c -- https://github.com/takeiteasy/fun-with-pixels

 fun-with-pixels

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

#define FWP_COMMAND_LINE
#include "fwp.h"

#define PLATFORM_POSIX
#if defined(__APPLE__) || defined(__MACH__)
#define PLATFORM_MAC
#elif defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WINDOWS
#if !defined(PLATFORM_FORCE_POSIX)
#undef PLATFORM_POSIX
#endif
#elif defined(__gnu_linux__) || defined(__linux__) || defined(__unix__)
#define PLATFORM_LINUX
#else
#error Unknown platform
#endif

#if defined(PLATFORM_WINDOWS)
#include <Windows.h>
#include <io.h>
#define F_OK 0
#define access _access
#include "getopt_win32.h"
#ifndef _MSC_VER
#pragma comment(lib, "Psapi.lib")
#endif
#define DLFCN_IMPLEMENTATION
#include "dlfcn_win32.h"
#else // PLATFORM_POSIX
#include <getopt.h>
#define _BSD_SOURCE // usleep()
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct {
#if defined(PLATFORM_WINDOWS)
    FILETIME writeTime;
#else
    ino_t handleID;
#endif
    void *handle;
    fwpState *state;
    fwpScene *scene;
    pbImage *buffer;
    struct {
        unsigned int width;
        unsigned int height;
        const char *title;
        pbFlags flags;
        char *path;
    } args;
} state;

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
    puts(" usage: fwp [path to dylib] [options]");
    puts("");
    puts(" fun-with-pixels  Copyright (C) 2024  George Watson");
    puts(" This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'.");
    puts(" This is free software, and you are welcome to redistribute it");
    puts(" under certain conditions; type `show c' for details.");
    puts("");
    puts("  Options:");
    puts("      -w/--width     Window width [default: 640]");
    puts("      -h/--height    Window height [default: 480]");
    puts("      -t/--title     Window title [default: \"fwp\"]");
    puts("      -r/--resizable Enable resizable window");
    puts("      -a/--top       Enable window always on top");
    puts("      -u/--usage     Display this message");
}

#if defined(PLATFORM_WINDOWS)
static FILETIME Win32GetLastWriteTime(char* path) {
    FILETIME time;
    WIN32_FILE_ATTRIBUTE_DATA data;

    if (GetFileAttributesEx(path, GetFileExInfoStandard, &data))
        time = data.ftLastWriteTime;

    return time;
}
#endif

static int ShouldReloadLibrary(void) {
#if defined(PLATFORM_WINDOWS)
    FILETIME newTime = Win32GetLastWriteTime(state.args.path);
    int result = CompareFileTime(&newTime, &state.writeTime);
    if (result)
        state.writeTime = newTime;
    return result;
#else
    struct stat attr;
    int result = !stat(state.args.path, &attr) && state.handleID != attr.st_ino;
    if (result)
        state.handleID = attr.st_ino;
    return result;
#endif
}

#if defined(PLATFORM_WINDOWS)
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

    if (state.handle) {
        if (state.scene->unload)
            state.scene->unload(state.state);
        dlclose(state.handle);
    }

#if defined(PLATFORM_WINDOWS)
    size_t newPathSize = strlen(path) + 4;
    char *newPath = malloc(sizeof(char) * newPathSize);
    char *noExt = RemoveExt(path);
    sprintf(newPath, "%s.tmp.dll", noExt);
    CopyFile(path, newPath, 0);
    handle = dlopen(newPath, RTLD_NOW);
    free(newPath);
    free(noExt);
    if (!state.handle)
#else
    if (!(state.handle = dlopen(path, RTLD_NOW)))
#endif
        goto BAIL;
    if (!(state.scene = dlsym(state.handle, "scene")))
        goto BAIL;
    if (!state.state) {
        if (state.scene->windowWidth > 0 && state.scene->windowHeight > 0)
            pbSetWindowSize(state.scene->windowWidth, state.scene->windowHeight);
        if (state.scene->windowTitle)
            pbSetWindowTitle(state.scene->windowTitle);
        if (!(state.state = state.scene->init(state.buffer)))
            goto BAIL;
    } else {
        if (state.scene->windowWidth > 0 && state.scene->windowHeight > 0)
            pbSetWindowSize(state.scene->windowWidth, state.scene->windowHeight);
        if (state.scene->windowTitle)
            pbSetWindowTitle(state.scene->windowTitle);
        if (state.scene->reload)
            state.scene->reload(state.state);
    }
    return 1;

BAIL:
    if (state.handle)
        dlclose(state.handle);
    state.handle = NULL;
#if defined(PLATFORM_WINDOWS)
    memset(&state.writeTime, 0, sizeof(FILETIME));
#else
    state.handleID = 0;
#endif
    return 0;
}

#define pbInputCallback(E)                    \
    if (state.scene->event)                   \
        state.scene->event(state.state, &(E)) \

static void pbInputKeyboard(void *userdata, int key, int modifier, int isDown) {
    pbEvent e = {
        .type = KeyboardEvent,
        .keyboard = {
            .key = key,
            .isdown = isDown
        },
        .modifier = modifier
    };
    pbInputCallback(e);
}

static void pbInputMouseButton(void *userdata, int button, int modifier, int isDown) {
    pbEvent e = {
        .type = MouseButtonEvent,
        .mouse = {
            .button = button,
            .isdown = isDown
        },
        .modifier = modifier
    };
    pbInputCallback(e);
}

static void pbInputMouseMove(void *userdata, int x, int y, float dx, float dy) {
    pbEvent e = {
        .type = MouseMoveEvent,
        .mouse = {
            .position = {
                .x = x,
                .y = y,
                .dx = dx,
                .dy = dy
            }
        }
    };
    pbInputCallback(e);
}

static void pbInputMouseScroll(void *userdata, float dx, float dy, int modifier) {
    pbEvent e = {
        .type = MouseScrollEvent,
        .mouse = {
            .wheel = {
                .dx = dx,
                .dy = dy
            }
        }
    };
    pbInputCallback(e);
}

static void pbInputFocus(void *userdata, int isFocused) {
    pbEvent e = {
        .type = FocusEvent,
        .window = {
            .focused = isFocused
        }
    };
    pbInputCallback(e);
}

static void pbInputResized(void *userdata, int w, int h) {
    pbEvent e = {
        .type = ResizedEvent,
        .window = {
            .size = {
                .width = w,
                .height = h
            }
        }
    };
    pbInputCallback(e);
}

static void pbInputClosed(void *userdata) {
    pbEvent e = {
        .type = ClosedEvent,
        .window = {
            .closed = true
        }
    };
    pbInputCallback(e);
}

int main(int argc, char *argv[]) {
    extern char* optarg;
    extern int optopt;
    extern int optind;
    int opt;
    while ((opt = getopt_long(argc, argv, ":w:h:t:uar", long_options, NULL)) != -1) {
        switch (opt) {
            case 'w':
                state.args.width = atoi(optarg);
                break;
            case 'h':
                state.args.height = atoi(optarg);
                break;
            case 't':
                state.args.title = optarg;
                break;
            case 'r':
                state.args.flags |= pbResizable;
                break;
            case 'a':
                state.args.flags |= pbAlwaysOnTop;
                break;
            case ':':
                printf("ERROR: \"-%c\" requires an value!\n", optopt);
                usage();
                return 0;
            case '?':
                printf("ERROR: Unknown argument \"-%c\"\n", optopt);
                usage();
                return 0;
            case 'u':
                usage();
                return 1;
            default:
                usage();
                return 0;
        }
    }

    if (optind >= argc) {
        puts("ERROR: No path to dynamic library provided");
        usage();
        return 0;
    } else {
        state.args.path = argv[optind];
    }
    if (state.args.path) {
#if !defined(PLATFORM_WINDOWS)
        if (state.args.path[0] != '.' || state.args.path[1] != '/') {
            char *tmp = malloc(strlen(state.args.path) + 2 * sizeof(char));
            sprintf(tmp, "./%s", state.args.path);
            state.args.path = tmp;
        } else
            state.args.path = strdup(state.args.path);
#endif
    }

    if (access(state.args.path, F_OK)) {
        printf("ERROR: No file found at path \"%s\"\n", state.args.path);
        return 0;
    }

    if (!state.args.width)
        state.args.width = 640;
    if (!state.args.height)
        state.args.height = 480;
    if (!state.args.title)
        state.args.title = "fwp";
    pbBegin(state.args.width, state.args.height, state.args.title, state.args.flags);

    if (!(state.buffer = pbImageNew(state.args.width, state.args.height)))
        return 0;

    if (!ReloadLibrary(state.args.path))
        return 0;

#define X(NAME, _) pbInput##NAME,
    pbCallbacks(FWP_PB_CALLBACKS NULL);
#undef X

    while (pbPoll()) {
        if (!ReloadLibrary(state.args.path))
            break;
        if (!state.scene->tick(state.state, state.buffer, 0.f))
            break;
        pbFlush(state.buffer);
    }

    state.scene->deinit(state.state);
    if (state.handle)
        dlclose(state.handle);
    pbImageFree(state.buffer);
#if !defined(PLATFORM_WINDOWS)
    free(state.args.path);
#endif
    pbEnd();
    return 1;
}
