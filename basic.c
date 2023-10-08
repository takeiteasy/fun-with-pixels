// basic.c - A simple example of how to use pp
#include "pp.h"
#if defined(PP_EMSCRIPTEN)
#include <emscripten.h>
#endif
#include <stdio.h>
#include <stdlib.h>

void onKeyboard(void *userdata, int key, int modifier, int isDown) {
    printf("Keyboard Event: Key %d is now %s\n", (int)key, isDown ? "down" : "up");
}

void onMouseButton(void *userdata, int button, int modifier, int isDown) {
    printf("Mouse Button Event: Button %d is now %s\n", button, isDown ? "down" : "up");
}

void onMouseMove(void *userdata, int x, int y, float dx, float dy) {
    printf("Mouse Move Event: Position (%d, %d) by (%f, %f)\n", x, y, dx, dy);
}

void onMouseScroll(void *userdata, float dx, float dy, int modifier) {
    printf("Mouse Scroll Event: Scroll delta (%f, %f)\n", dx, dy);
}

void onFocus(void *userdata, int isFocused) {
    printf("Focus Event: Window is now %s\n", isFocused ? "focused" : "unfocused");
}

void onResized(void *userdata, int w, int h) {
    printf("Resize Event: Window is now (%d, %d)\n", w, h);
}

void onClosed(void *userdata) {
    printf("Close Event: Window is now closing\n");
}

static int *test = NULL;

static void loop(void) {
    ppFlush(test, 640, 480);
}

int main(int argc, const char *argv[]) {
    ppBegin(640, 480, "pp", ppResizable);
#define X(NAME, ARGS) on##NAME,
    ppCallbacks(PP_CALLBACKS NULL);
#undef X
    
    test = (int*)malloc(sizeof(int) * 640 * 480);
    for (int x = 0; x < 640; x++)
        for (int y = 0; y < 480; y++)
            test[y * 640 + x] = 0xFFFF0000;
    
#if defined(PP_EMSCRIPTEN)
    emscripten_set_main_loop(loop, 0, 1);
#else
    while (ppPoll())
        loop();
#endif
    
    free(test);
    ppEnd();
    return 0;
}
