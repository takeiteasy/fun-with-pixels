/* ppLive.c -- https://github.com/takeiteasy/pp
 
 Live coding enviroment, inspired by https://nullprogram.com/blog/2014/12/23/ and Handmade Hero (https://youtu.be/WMSBRk5WG58)
 
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

#if defined(PP_EMSCRIPTEN)
#undef PP_EMSCRIPTEN
#endif
#include "pp.h"
#define PP_INPUT_IMPLEMENTATION
#include "ppInput.h"
#include "ppLive.h"

static State *state = NULL;
static App *app = NULL;
static struct {
    int width;
    int height;
    const char *title;
    ppFlags flags;
    int clearColor;
    char *path;
} Args = {0};
static Bitmap pbo;

#if defined(PP_WINDOWS)
#define _WIN32_WINNT 0x0550
#include <windows.h>
#if defined(_MSC_VER)
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "ole32.lib")
#pragma warning (disable: 4996)
#endif
#include <io.h>
#define F_OK 0
#define access _access
#define SLEEP(ms) Sleep(ms)
#define strdup(STR) _strdup(STR)
static HINSTANCE handle = NULL;
#define LOAD_LIBRARY(PATH) LoadLibraryA(PATH)
#define LIBRARY_SYM(HANDLE, SYM) GetProcAddress(HANDLE, SYM)
#define CLOSE_LIBRARY(HANDLE) \
    if (HANDLE)               \
        FreeLibrary(HANDLE)
#error Windows is not yet implemented!
#else // POSIX
#include <getopt.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <dlfcn.h>
#define SLEEP(ms) usleep(ms * 1000)
static void *handle = NULL;
#define LOAD_LIBRARY(PATH) dlopen(PATH, RTLD_NOW)
#define LIBRARY_SYM(HANDLE, SYM) dlsym(HANDLE, SYM)
#define CLOSE_LIBRARY(HANDLE) \
    if (HANDLE)               \
        dlclose(HANDLE)

#if defined(PP_MAC)
#include <sys/event.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdatomic.h>
static int kq, fd;
static struct kevent ke;
static _Atomic bool threadQuit = false;
static _Atomic bool reloadingLibrary = false;
#elif defined(PP_LINUX)
#include <sys/inotify.h>
#error Linux is not yet implemented!
#else
#error This operating system is unsupported by ppLive! Sorry!
#endif
#endif

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
    puts("usage: ppLive -p [path] [options]\n");
    puts("\t-w/--width\tWindow width\t\t [default: 640]");
    puts("\t-h/--height\tWindow height\t\t [default: 480]");
    puts("\t-t/--title\tWindow title\t\t [default: \"pp\"]");
    puts("\t-r/--resizable\tEnable resizable window");
    puts("\t-a/--top\tEnable window always on top");
    puts("\t-p/--path\tPath the dynamic library [required]");
    puts("\t-u/--usage\tDisplay this message");
}

static bool loadLibrary(void) {
    if (handle) {
        app->unload(state);
        CLOSE_LIBRARY(handle);
    }
    
    if (!(handle = LOAD_LIBRARY(Args.path))) {
        printf("ERROR: Failed to open \"%s\"!", Args.path);
        return false;
    }
    if (!(app = LIBRARY_SYM(handle, "pp"))) {
        puts("ERROR: Failed to find \"pp\" symbol!");
        CLOSE_LIBRARY(handle);
        return false;
    }
    if (!state) {
        if (!(state = app->init())) {
            puts("ERROR: Library init function return NULL!");
            CLOSE_LIBRARY(handle);
            return false;
        }
    } else
        app->reload(state);
    return true;
}

static bool startMonitor(void) {
    if (!(fd = open(Args.path, O_RDONLY))) {
        printf("ERROR: An error occured while opening \"%s\"!", Args.path);
        return false;
    }
    if (!(kq = kqueue())) {
        puts("ERROR: An error occured creating kqueue!");
        return false;
    }
    EV_SET(&ke, fd, EVFILT_VNODE, EV_ADD | EV_CLEAR, NOTE_DELETE | NOTE_WRITE, 0, NULL);
    if (kevent(kq, &ke, 1, NULL, 0, NULL) == -1) {
        puts("ERROR: An error occured with kevent!");
        return false;
    }
    return true;
}

static void endMonitor(void) {
    close(kq);
    close(fd);
}

static void* monitorFunc(void *arg) {
#define BAIL               \
    do {                   \
        threadQuit = true; \
        return NULL;       \
    } while (0)

    if (!startMonitor())
        BAIL;
    
    struct timespec to = {0};
    while (!threadQuit) {
        int r;
        if ((r = kevent(kq, NULL, 0, &ke, 1, NULL)) == -1)
            BAIL;
        
        do {
            if ((r = kevent(kq, NULL, 0, &ke, 1, &to)) == -1)
                BAIL;
        } while (r);
        
        if (ke.fflags & NOTE_DELETE) {
            endMonitor();
            while (access(Args.path, F_OK))
                SLEEP(1);
            if (!startMonitor())
                BAIL;
        } else if (ke.fflags & NOTE_WRITE) {
            reloadingLibrary = true;
            if (!loadLibrary())
                BAIL;
            reloadingLibrary = false;
        }
    }
    return NULL;
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
                return EXIT_FAILURE;
            case '?':
                printf("ERROR: Unknown argument \"-%c\"\n", optopt);
                usage();
                return EXIT_FAILURE;
            case 'u':
                usage();
                return EXIT_SUCCESS;
            default:
                usage();
                return EXIT_FAILURE;
        }
    }
    
    if (!Args.path) {
        puts("ERROR: No path to dynamic library provided (-p/--path)");
        usage();
        return EXIT_FAILURE;
    } else {
        if (Args.path[0] != '.' || Args.path[1] != '/') {
            char *tmp = malloc(strlen(Args.path) + 2 * sizeof(char));
            sprintf(tmp, "./%s", Args.path);
            Args.path = tmp;
        } else
            Args.path = strdup(Args.path);
    }
    if (access(Args.path, F_OK)) {
        printf("ERROR: No file found at path \"%s\"\n", Args.path);
        return EXIT_FAILURE;
    }
    
    if (!Args.width)
        Args.width = 640;
    if (!Args.height)
        Args.height = 480;
    if (!Args.clearColor)
        Args.clearColor = Black;
    
    ppBegin(Args.width, Args.height, Args.title ? Args.title : "pp", Args.flags);
    ppInitInput();
    InitBitmap(&pbo, Args.width, Args.height);
    
    int i, noise, carry, seed = 0xBEEF;
    Bitmap loading;
    InitBitmap(&loading, Args.width, Args.height);
    
    if (!loadLibrary())
        return EXIT_FAILURE;
    
    pthread_t monitorThread;
    pthread_create(&monitorThread, NULL, monitorFunc, NULL);
    
    double lastTime = ppTime();
    while (ppPoll() && !threadQuit) {
        double now = ppTime();
        double delta = now - lastTime;
        lastTime = now;
        
        if (!reloadingLibrary) {
            if (!app->tick(state, &pbo, delta))
                break;
            ppFlush(&pbo);
        } else {
            // https://github.com/emoon/minifb/blob/master/tests/noise.c
            for (int i = 0; i < Args.width * Args.height; i++) {
                noise = seed;
                noise >>= 3;
                noise ^= seed;
                carry = noise & 1;
                noise >>= 1;
                seed >>= 1;
                seed |= (carry << 30);
                noise &= 0xFF;
                loading.buf[i] = RGB1(noise);
            }
            DrawString(&loading, "Loading...", 0, 0, White);
            ppFlush(&loading);
        }
    }
    
    app->deinit(state);
    threadQuit = true;
    endMonitor();
    CLOSE_LIBRARY(handle);
    DestroyBitmap(&loading);
    DestroyBitmap(&pbo);
    free(Args.path);
    ppEnd();
    return EXIT_SUCCESS;
}
