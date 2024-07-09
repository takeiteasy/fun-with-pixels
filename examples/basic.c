// Firstly include the fwp header, which contains everything we need
#include "fwp.h"
#include <stdlib.h> // malloc

// A fwpState should be defined in each scene. This structure can contain whatever variables and types you want, but it must be defined like this. Do not typedef the struct definition, as it is already typedef'd in fwp.h
struct fwpState {
    pbColor clearColor;
};

static fwpState* init(pbImage *framebuffer) {
    // Called once when the program first starts
    // You must always create an instance of your fwpState definition
    // It must be allocated on the stack, not the heap
    // This object will be be used to keep track of things between reloads
    fwpState *state = malloc(sizeof(fwpState));
    state->clearColor = RGB(255, 0, 0);
    // Return your fwpState so fwp can keep track of it
    return state;
}

static void deinit(fwpState *state) {
    // Only called when the program is exiting
    if (state)
        free(state);
}

static void reload(fwpState *state) {
    // Called when the dynamic has been updated + reloaded
    // Here we change the `clearColor` field in our state to blue
    // If you rebuild the library, the screen will chang from red
    // to blue! Magic!
    state->clearColor = RGB(0, 0, 255);
}

static void unload(fwpState *state) {
    // Called when dynamic library has been unloaded
}

static int event(fwpState *state, pbEvent *e) {
    // Called on window event
    return 1;
}

static int tick(fwpState *state, pbImage *pbo, double delta) {
    // Called every frame, this is your update callback
    pbImageFill(pbo, state->clearColor);
    return 1;
}

// So fwp knows where your callbacks are a `scene` definition must be made
// The definition should be always be called scene. If the name changes fwp
// won't know where to look!
EXPORT const fwpScene scene = {
    .init = init,
    .deinit = deinit,
    .reload = reload,
    .unload = unload,
    .event = event,
    .tick = tick
};
