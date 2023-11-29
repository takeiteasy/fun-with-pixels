#include "ppLive.h"
#include <time.h>
#include <stdlib.h>

struct ppState {
    ppColor clearColor;
};

// Called once at the beginning of runtime
static ppState* init(void) {
    ppState *state = malloc(sizeof(ppState));
    if (!state)
        return NULL;
    state->clearColor = (ppColor){.r = 255, .g = 0, .b = 0};
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
}

// Called just before the app is reloaded
static void unload(ppState *state) {
}

static int event(ppState *state, ppEvent *e) {
    switch (e->type) {
        case ppKeyboardEvent:
            printf("Keyboard key %d is now %s\n", e->Keyboard.key, e->Keyboard.isdown ? "pressed" : "released");
            break;
        case ppMouseButtonEvent:
            printf("Mouse button %d is now %s\n", e->Mouse.button, e->Mouse.isdown ? "pressed" : "released");
            break;
        case ppMouseMoveEvent:
            printf("Mouse moved by %fx%f to %dx%d\n", e->Mouse.Position.dx, e->Mouse.Position.dy, e->Mouse.Position.x, e->Mouse.Position.y);
            break;
        case ppMouseScrollEvent:
            printf("Mouse scrolled by %fx%f\n", e->Mouse.Scroll.dx, e->Mouse.Scroll.dy);
            break;
        case ppResizedEvent:
            printf("Window has resized to %dx%d\n", e->Window.Size.width, e->Window.Size.height);
            break;
        case ppFocusEvent:
            printf("Window is now %s\n", e->Window.focused ? "focused" : "blured");
            break;
        case ppClosedEvent:
            printf("Window closed\n");
            return 0;
    }
    return 1;
}

// Called every frame
static int tick(ppState *state, ppSurface *frame, double delta) {
    ppClearSurface(frame, state->clearColor);
    return 1;
}

// App descriptor to direct callbacks
// Must be called "pp" as that is the symbol that is looked up
EXPORT const ppApp pp = {
    .init = init,
    .deinit = deinit,
    .reload = reload,
    .unload = unload,
    .event = event,
    .tick = tick
};
