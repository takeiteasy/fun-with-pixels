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
    state->clearColor = RGB(ppRandomInt(256), ppRandomInt(256), ppRandomInt(256));
}

// Called just before the app is reloaded
static void unload(ppState *state) {
}

static bool event(ppState *state, ppEvent *e) {
    switch (e->type) {
        case KeyboardEvent:
            printf("EVENT: Keyboard key %d is now %s\n", e->Keyboard.key, e->Keyboard.isdown ? "pressed" : "released");
            break;
        case MouseButtonEvent:
            printf("EVENT: Mouse button %d is now %s\n", e->Mouse.button, e->Mouse.isdown ? "pressed" : "released");
            break;
        case MouseMoveEvent:
            printf("EVENT: Mouse moved by %fx%f to %dx%d\n", e->Mouse.Position.dx, e->Mouse.Position.dy, e->Mouse.Position.x, e->Mouse.Position.y);
            break;
        case MouseScrollEvent:
            printf("EVENT: Mouse scrolled by %fx%f\n", e->Mouse.Scroll.dx, e->Mouse.Scroll.dy);
            break;
        case ResizedEvent:
            printf("EVENT: Window has resized to %dx%d\n", e->Window.Size.width, e->Window.Size.height);
            break;
        case FocusEvent:
            printf("EVENT: Window is now %s\n", e->Window.focused ? "focused" : "blured");
            break;
        case ClosedEvent:
            printf("EVENT: Window closed\n");
            return false;
    }
    return true;
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
EXPORT const ppApp pp = {
    .init = init,
    .deinit = deinit,
    .reload = reload,
    .unload = unload,
    .event = event,
    .tick = tick
};
