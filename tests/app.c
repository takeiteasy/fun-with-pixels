#define PP_APP_IMPLEMENTATION
#define PP_INPUT_IMPLEMENTATION
#include "ppApp.h"

// Called after window is created before loop starts
static bool init(void) {
    // Initalize anything you need here
    return true; // return false to exit early
}

// Called every frame after events are polled and framebuffer is cleared
// Return false to force exit loop or true to keep looping
// `pbo`   is the framebuffer that is drawn to the window
// `delta` is the delta time between frames
static bool tick(Bitmap *pbo, double delta) {
    if (ppIsKeyDown(KEY_ESCAPE)) // Press escape key to quit
        return false;
    DrawStringFormat(pbo, 0, 0, White, "Frame Delta: %f", delta);
    return true;
}

// Called at exit after loop is exited
static void deinit(void) {
    // Free up any allocated memory here
}

// Application entry, return descriptor to begin
ppApp Main(int argc, const char *argv[]) {
    return (ppApp) {
        .width = 640,         // Window width (must be set)
        .height = 480,        // Window height (must be set)
//      .fbWidth = 640,       // Framebuffer width (optional, defaults to window width)
//      .fbHeight = 480,      // Framebuffer height (optional, defaults to window height)
        .title = "ppApp",     // Window title (optional, defaults to "pp")
        .flags = ppResizable, // Window flags (optional)
        .clearColor = Red,    // Framebuffer clear color (defaults to black)
        .init = init,         // Init callback (optional)
        .tick = tick,         // Tick callback (must be set)
        .deinit = deinit      // Deinit callback (optional)
    };
}
