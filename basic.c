// basic.c - A simple example of how to use pp
#include "pp.h"

#define WIDTH 640
#define HEIGHT 480

// Window event callbacks, I think the names are self-explanatory
void onKeyboard(void *userdata, ppKey key, ppMod modifier, bool isDown) {
    printf("Keyboard Event: Key %d is now %s\n", (int)key, isDown ? "down" : "up");
}

void onMouseButton(void *userdata, int button, ppMod modifier, bool isDown) {
    printf("Mouse Button Event: Button %d is now %s\n", button, isDown ? "down" : "up");
}

void onMouseMove(void *userdata, int x, int y, float dx, float dy) {
    printf("Mouse Move Event: Position (%d, %d) by (%f, %f)\n", x, y, dx, dy);
}

void onMouseScroll(void *userdata, float dx, float dy, ppMod modifier) {
    printf("Mouse Scroll Event: Scroll delta (%f, %f)\n", dx, dy);
}

void onFocus(void *userdata, bool isFocused) {
    printf("Focus Event: Window is now %s\n", isFocused ? "focused" : "unfocused");
}

void onResized(void *userdata, int w, int h) {
    printf("Resize Event: Window is now (%d, %d)\n", w, h);
}

void onClosed(void *userdata) {
    printf("Close Event: Window is now closing\n");
}

static Bitmap screen;

static void loop(void) {
    for (int x = 0; x < WIDTH; x++)
        for (int y = 0; y < HEIGHT; y++)
            PSet(&screen, x, y, RGBA(255, 0, 0, 255));
    ppFlush(&screen);
}

int main(int argc, const char *argv[]) {
    ppBegin(WIDTH, HEIGHT, "pp", ppResizable);
#define X(NAME, ARGS) on##NAME,
    ppCallbacks(PP_CALLBACKS NULL);
#undef X
    
    InitBitmap(&screen, WIDTH, HEIGHT);
    
#if defined(PP_EMSCRIPTEN)
    emscripten_set_main_loop(loop, 0, 1);
#else
    while (ppPoll())
        loop();
#endif
    
    ppEnd();
    return 0;
}
