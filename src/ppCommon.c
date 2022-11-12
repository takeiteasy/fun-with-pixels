#include "pp.h"
#define BITMAP_IMPLEMENTATION
#define BITMAP_ENABLE_IO
#define BITMAP_ENABLE_DEBUG_FONT
#define BITMAP_ENABLE_BDF_FONT
#define BITMAP_ENABLE_TTF_FONT
#define BITMAP_ENABLE_GIFS
#include "bitmap.h"

static struct {
#define X(NAME, ARGS) void(*NAME##Callback)ARGS;
    PP_CALLBACKS
#undef X
    void *userdata;
    bool initialized;
    bool running;
} ppInternal = {0};

#define X(NAME, ARGS) \
    void(*NAME##Callback)ARGS,
void ppWindowCallbacks(PP_CALLBACKS void* userdata) {
#undef X
#define X(NAME, ARGS) \
    ppInternal.NAME##Callback = NAME##Callback;
  PP_CALLBACKS
#undef X
    ppInternal.userdata = userdata;
}

#define X(NAME, ARGS) \
    void ppWindow##NAME##Callback(void(*NAME##Callback)ARGS) { \
        ppInternal.NAME##Callback = NAME##Callback; \
    }
PP_CALLBACKS
#undef X

void ppWindowUserdata(void *userdata) {
    ppInternal.userdata = userdata;
}

bool ppWindowRunning() {
    return ppInternal.running;
}
