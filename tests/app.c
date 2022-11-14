#define PP_APP_IMPLEMENTATION
#include "ppApp.h"

static bool init(void) {
    printf("Init\n");
    return true;
}

static void preFrame(void) {
    printf("Pre Frame\n");
}

static bool tick(double delta) {
    printf("Tick: %f\n", delta);
    return true;
}

static bool fixedTick(double delta) {
    printf("Fixed Tick: %f\n", delta);
    return true;
}

static void render(double delta) {
    printf("Render: %f\n", delta);
}

static void postFrame(void) {
    printf("Post Frame\n");
}

static void deinit(void) {
    printf("Deinit\n");
}

ppApp Main(int argc, const char *argv[]) {
    return (ppApp) {
        .width = 640,
        .height = 480,
        .title = "ppApp",
        .flags = ppResizable,
        .targetFPS = 60.0,
        .unlockFramerate = true,
        .clearColor = Red,
        .init = init,
        .preFrame = preFrame,
        .tick = tick,
        .fixedTick = fixedTick,
        .render = render,
        .postFrame = postFrame,
        .deinit = deinit
    };
}
