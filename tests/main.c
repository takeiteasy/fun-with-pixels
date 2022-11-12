#include "pp.h"

#define WIDTH  640
#define HEIGHT 480

int main(int argc, const char *argv[]) {
    ppWindow(WIDTH, HEIGHT, "test", ppResizable);
    
    Bitmap pbo;
    InitBitmap(&pbo, WIDTH, HEIGHT);
    
    while (ppPollEvents()) {
        ppFlush(&pbo);
    }
    
    DestroyBitmap(&pbo);
    ppRelease();
    
    return 0;
}
