#include "ppLive.h"
#include <time.h>
#include <stdlib.h>

struct State {
    int clearColor;
};

// Called once at the beginning of runtime
static State* init(void) {
    State *state = malloc(sizeof(State));
    if (!state)
        return NULL;
    state->clearColor = Black;
    srand((unsigned int)time(NULL));
    return state;
}

// Called once at the end of runtime
static void deinit(State *state) {
    if (!state)
        return;
    free(state);
}

// Called whenever the app is modified and reloaded
static void reload(State *state) {
    state->clearColor = RGB(rand() % 256, rand() % 256, rand() % 256);
}

// Called just before the app is reloaded
static void unload(State *state) {
}

// Called every frame
static bool tick(State *state, Bitmap *pbo, double delta) {
    FillBitmap(pbo, state->clearColor);
    DrawString(pbo, "Rebuild to change the background color!", 1, 1, White);
    DrawRect(pbo, 50, 50, 50, 50, Red, true);
    return true;
}

// App descriptor to
const App pp = {
    .init = init,
    .deinit = deinit,
    .reload = reload,
    .unload = unload,
    .tick = tick
};
