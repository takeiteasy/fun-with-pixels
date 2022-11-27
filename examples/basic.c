// basic.c - A simple example of how to use pp
// For infomation on bitmaps, see: https://github.com/takeiteasy/bitmap
#include "pp.h"

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

#define WIDTH  640
#define HEIGHT 480
static Bitmap pbo;

static void loop(void) {
    // Fill framebuffer bitmap solid red
    FillBitmap(&pbo, Red);
    // Draw a string to framebuffer using in-built debug font
    DrawStringFormat(&pbo, 0, 0, White, "Frame Delta: %f", ppTime());
    // Draw a blue rectangle to framebuffer
    DrawRect(&pbo, 50, 50, 50, 50, Blue, true);
    // Draw framebuffer to screen
    ppFlush(&pbo);
}

int main(int argc, const char *argv[]) {
    // Create new resizable window
    ppBegin(WIDTH, HEIGHT, "pp", ppResizable);
    // Assign all event callbacks (see above)
    // Callbacks can be assigned individually or all at once
    // The NULL value is optional userdata that will be passed to the event callbacks
#define X(NAME, ARGS) on##NAME,
    ppCallbacks(PP_CALLBACKS NULL);
#undef X
    
    // Create empty bitmap to act as framebuffer
    // The "framebuffer" can be any size and will resize to fit the window
    InitBitmap(&pbo, WIDTH, HEIGHT);
    
    // ppPoll collects all window events and will return true while window is open
#if defined(PP_EMSCRIPTEN)
    emscripten_set_main_loop(loop, 0, 1);
#else
    while (ppPoll())
        loop();
#endif
    
    // Free bitmap
    DestroyBitmap(&pbo);
    // Free window
    ppEnd();
    return 0;
}
