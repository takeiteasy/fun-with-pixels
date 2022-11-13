#include "pp.h"

#define WIDTH  640
#define HEIGHT 480

int main(int argc, const char *argv[]) {
    ppWindow(WIDTH, HEIGHT, "test", ppResizable);
    
    Bitmap pbo;
    InitBitmap(&pbo, WIDTH, HEIGHT);
    FillBitmap(&pbo, Red);
    
    DrawRect(&pbo, 10, 10, 50, 50, Blue, true);
    
    while (ppPollEvents()) {
        ppFlush(&pbo);
    }
    
    DestroyBitmap(&pbo);
    ppRelease();
    
    return 0;
}
