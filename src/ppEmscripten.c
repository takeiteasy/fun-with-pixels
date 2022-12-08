/* ppEmscripten.c -- https://github.com/takeiteasy/pp
 
 The MIT License (MIT)

 Copyright (c) 2022 George Watson

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software,
 and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "ppCommon.c"
#include <emscripten.h>
#include <emscripten/html5.h>

#define canvas "#canvas"
static struct {
    int screenW, screenH;
    int canvasW, canvasH;
    bool mouseInCanvas;
} ppEmccInternal = {0};

static ppMod TranslateWebMod(bool ctrl, bool shift, bool alt, bool meta) {
    return ((ctrl ? KEY_MOD_CONTROL : 0) | (shift ? KEY_MOD_SHIFT : 0) | (alt ? KEY_MOD_ALT : 0) | (meta ? KEY_MOD_SUPER : 0));
}

static EM_BOOL key_callback(int type, const EmscriptenKeyboardEvent* e, void* user_data) {
    ppMod mod = TranslateWebMod(e->ctrlKey, e->shiftKey, e->altKey, e->metaKey);
    ppCallCallback(Keyboard, e->keyCode, mod, type == EMSCRIPTEN_EVENT_KEYDOWN);
    return e->keyCode == 82 && mod == KEY_MOD_SUPER ? false : true;
}

static EM_BOOL mouse_callback(int type, const EmscriptenMouseEvent* e, void* user_data) {
    switch (type) {
        case EMSCRIPTEN_EVENT_MOUSEDOWN:
            if (ppEmccInternal.mouseInCanvas && e->buttons != 0)
                ppCallCallback(MouseButton, e->button + 1, TranslateWebMod(e->ctrlKey, e->shiftKey, e->altKey, e->metaKey), true);
            break;
        case EMSCRIPTEN_EVENT_MOUSEUP:
            if (ppEmccInternal.mouseInCanvas)
                ppCallCallback(MouseButton, e->button + 1, TranslateWebMod(e->ctrlKey, e->shiftKey, e->altKey, e->metaKey), false);
            break;
        case EMSCRIPTEN_EVENT_MOUSEMOVE:
            if (ppEmccInternal.mouseInCanvas)
                ppCallCallback(MouseMove, e->clientX - (ppEmccInternal.screenW / 2) + (ppEmccInternal.canvasW / 2), e->clientY, e->movementX, e->movementY);
            break;
        case EMSCRIPTEN_EVENT_MOUSEENTER:
            ppEmccInternal.mouseInCanvas = true;
            return true;
        case EMSCRIPTEN_EVENT_MOUSELEAVE:
            ppEmccInternal.mouseInCanvas = false;
            return false;
        case EMSCRIPTEN_EVENT_CLICK:
        case EMSCRIPTEN_EVENT_DBLCLICK:
        default:
            return false;
    }
    return true;
}

static EM_BOOL wheel_callback(int type, const EmscriptenWheelEvent* e, void* user_data) {
    ppCallCallback(MouseScroll, e->deltaX, e->deltaY, TranslateWebMod(e->mouse.ctrlKey, e->mouse.shiftKey, e->mouse.altKey, e->mouse.metaKey));
    return true;
}

static EM_BOOL uievent_callback(int type, const EmscriptenUiEvent* e, void* user_data) {
    ppEmccInternal.screenW = e->documentBodyClientWidth;
    ppEmccInternal.screenH = e->documentBodyClientHeight;
    emscripten_get_element_css_size(canvas, (double*)&ppEmccInternal.canvasW, (double*)&ppEmccInternal.canvasH);
    ppCallCallback(Resized, ppEmccInternal.screenW, ppEmccInternal.screenH);
    return true;
}

static EM_BOOL focusevent_callback(int type, const EmscriptenFocusEvent* e, void* user_data) {
    ppCallCallback(Focus, type == EMSCRIPTEN_EVENT_FOCUS);
    return true;
}

static const char* beforeunload_callback(int eventType, const void *reserved, void *userData) {
    return "Do you really want to leave the page?";
}

static bool ppBeginNative(int w, int h, const char *title, ppFlags flags) {
    emscripten_set_canvas_element_size(canvas, w, h);
    
    emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, key_callback);
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, key_callback);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, key_callback);
    
    emscripten_set_click_callback(canvas, 0, 1, mouse_callback);
    emscripten_set_mousedown_callback(canvas, 0, 1, mouse_callback);
    emscripten_set_mouseup_callback(canvas, 0, 1, mouse_callback);
    emscripten_set_dblclick_callback(canvas, 0, 1, mouse_callback);
    emscripten_set_mousemove_callback(canvas, 0, 1, mouse_callback);
    emscripten_set_mouseenter_callback(canvas, 0, 1, mouse_callback);
    emscripten_set_mouseleave_callback(canvas, 0, 1, mouse_callback);
    emscripten_set_mouseover_callback(canvas, 0, 1, mouse_callback);
    emscripten_set_mouseout_callback(canvas, 0, 1, mouse_callback);
    
    emscripten_set_wheel_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, wheel_callback);

    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, uievent_callback);
    emscripten_set_scroll_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, uievent_callback);
    
    emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, focusevent_callback);
    emscripten_set_focus_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, focusevent_callback);
    emscripten_set_focusin_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, focusevent_callback);
    emscripten_set_focusout_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, focusevent_callback);
    
    ppInternal.running = true;
    return true;
}

bool ppPoll(void) {
    return true;
}

void ppFlush(Bitmap *bitmap) {
    if (!bitmap || !bitmap->buf || !bitmap->w || !bitmap->h) {
        ppInternal.pbo = NULL;
        return;
    }
    EM_ASM({
        var w = $0;
        var h = $1;
        var buf = $2;
        var src = buf >> 2;
        var canvas = document.getElementById("canvas");
        var ctx = canvas.getContext("2d");
        var img = ctx.createImageData(w, h);
        var data = img.data;
        
        var i = 0;
        var j = data.length;
        while (i < j) {
            var val = HEAP32[src];
            data[i  ] = (val >> 16) & 0xFF;
            data[i+1] = (val >> 8) & 0xFF;
            data[i+2] = val & 0xFF;
            data[i+3] = 0xFF;
            src++;
            i += 4;
        }
        
        ctx.putImageData(img, 0, 0);
    }, bitmap->w, bitmap->h, bitmap->buf);
}

void ppEnd(void) {
    // ...
}

double ppTime(void) {
    static double last = 0;
    if (!last)
        last = emscripten_get_now();
    double now = emscripten_get_now();
    double elapsed = last - emscripten_get_now();
    last = now;
    return elapsed;
}
