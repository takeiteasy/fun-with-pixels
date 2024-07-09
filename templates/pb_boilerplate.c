#include "pb.h"

int main(int argc, const char *argv[]) {
    // Initalize a new window titled "hello!"
    pbBegin(640, 480, "hello!", 0);
    // Create an image for the framebuffer
    pbImage *fb = pbImageNew(640, 480);
    // Loop while the window is open + poll events
    while (pbPoll()) {
        // Fill the image buffer red
        pbImageFill(fb, RGB(255, 0, 0));
        // Flush the image to the window's framebuffer
        pbFlush(fb);
    }
    // Clean up
    pbEnd();
    return 0;
}
