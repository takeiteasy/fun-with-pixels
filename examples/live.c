#include "pp.h"
#include <time.h>
#include <stdlib.h>

struct ppState {
    int clearColor;
};

// Called once at the beginning of runtime
static ppState* init(void) {
    ppState *state = malloc(sizeof(ppState));
    if (!state)
        return NULL;
    state->clearColor = Black;
    srand((unsigned int)time(NULL));
    return state;
}

// Called once at the end of runtime
static void deinit(ppState *state) {
    if (!state)
        return;
    free(state);
}

// Called whenever the app is modified and reloaded
static void reload(ppState *state) {
    state->clearColor = RGB(rand() % 256, rand() % 256, rand() % 256);
}

// Called just before the app is reloaded
static void unload(ppState *state) {
}

// Called every frame
static bool tick(ppState *state, Bitmap *pbo, double delta) {
    FillBitmap(pbo, state->clearColor);
    DrawString(pbo, "Rebuild to change the background color!", 1, 1, White);
    DrawRect(pbo, 50, 50, 50, 50, Red, true);
    return true;
}

// App descriptor to direct callbacks
// Must be called "pp" as that is the symbol that is looked up
const ppLiveApp pp = {
    .init = init,
    .deinit = deinit,
    .reload = reload,
    .unload = unload,
    .tick = tick
};
