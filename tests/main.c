#include "pp.h"

#define WIDTH  640
#define HEIGHT 480

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

int main(int argc, const char *argv[]) {
    ppBegin(WIDTH, HEIGHT, "pp", ppResizable);
    ppCallbacks(
#define X(NAME, ARGS) on##NAME,
                      PP_CALLBACKS
#undef X
                      NULL);
    
    Bitmap pbo;
    InitBitmap(&pbo, WIDTH, HEIGHT);
    
    double lastTime = ppTime();
    while (ppPoll()) {
        double now = ppTime();
        double delta = now - lastTime;
        lastTime = now;
        
        FillBitmap(&pbo, Red);
        DrawStringFormat(&pbo, 0, 0, White, "Frame Delta: %f", delta);
        DrawRect(&pbo, 50, 50, 50, 50, Blue, true);
        ppFlush(&pbo);
    }
    
    DestroyBitmap(&pbo);
    ppEnd();
    return 0;
}
