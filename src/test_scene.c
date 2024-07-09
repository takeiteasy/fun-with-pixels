#include "fwp.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

struct fwpState {
    pbColor clearColor;
};

static fwpState* init(void) {
    fwpState *state = malloc(sizeof(fwpState));
    assert(state);
    state->clearColor = RGB(255, 0, 0);
    return state;
}

static void deinit(fwpState *state) {
    if (state)
        free(state);
}

static void reload(fwpState *state) {
    printf("reload\n");
}

static void unload(fwpState *state) {
    printf("unload\n");
}

static int event(fwpState *state, pbEvent *e) {
    switch (e->type) {
        case KeyboardEvent:
            printf("EVENT: Keyboard key %d is now %s\n", e->keyboard.key, e->keyboard.isdown ? "pressed" : "released");
            break;
        case MouseButtonEvent:
            printf("EVENT: Mouse button %d is now %s\n", e->mouse.button, e->mouse.isdown ? "pressed" : "released");
            break;
        case MouseMoveEvent:
            printf("EVENT: Mouse moved by %fx%f to %dx%d\n", e->mouse.position.dx, e->mouse.position.dy, e->mouse.position.x, e->mouse.position.y);
            break;
        case MouseScrollEvent:
            printf("EVENT: Mouse scrolled by %fx%f\n", e->mouse.wheel.dx, e->mouse.wheel.dy);
            break;
        case ResizedEvent:
            printf("EVENT: Window has resized to %dx%d\n", e->window.size.width, e->window.size.height);
            break;
        case FocusEvent:
            printf("EVENT: Window is now %s\n", e->window.focused ? "focused" : "blured");
            break;
        case ClosedEvent:
            printf("EVENT: Window closed\n");
            return 0;
    }
    return 1;
}

static int tick(fwpState *state, pbImage *pbo, double delta) {
    pbImageFill(pbo, RGB(255, 0, 0));
    return 1;
}

EXPORT const fwpScene scene = {
    .init = init,
    .deinit = deinit,
    .reload = reload,
    .unload = unload,
    .event = event,
    .tick = tick
};
