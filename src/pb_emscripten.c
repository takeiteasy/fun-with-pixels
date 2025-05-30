/* pb_emscripten.c -- https://github.com/takeiteasy/fun-with-pixels

 fun-with-pixels

 Copyright (C) 2024  George Watson

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#define FWP_PB_IMPLEMENTATION
#include "pb.h"
#include <emscripten.h>
#include <emscripten/html5.h>

#define canvas "#canvas"
static struct {
    int screenW, screenH;
    int canvasW, canvasH;
    int mouseInCanvas;
} pbEmccInternal = {0};

static int TranslateWebMod(int ctrl, int shift, int alt, int meta) {
    return (ctrl ? KEY_MOD_CONTROL : 0) | (shift ? KEY_MOD_SHIFT : 0) | (alt ? KEY_MOD_ALT : 0) | (meta ? KEY_MOD_SUPER : 0);
}

static EM_BOOL key_callback(int type, const EmscriptenKeyboardEvent* e, void* user_data) {
    int mod = TranslateWebMod(e->ctrlKey, e->shiftKey, e->altKey, e->metaKey);
    pbCallCallback(Keyboard, e->keyCode, mod, type == EMSCRIPTEN_EVENT_KEYDOWN);
    return !(e->keyCode == 82 && mod == KEY_MOD_SUPER);
}

static EM_BOOL mouse_callback(int type, const EmscriptenMouseEvent* e, void* user_data) {
    switch (type) {
        case EMSCRIPTEN_EVENT_MOUSEDOWN:
            if (pbEmccInternal.mouseInCanvas && e->buttons != 0)
                pbCallCallback(MouseButton, e->button + 1, TranslateWebMod(e->ctrlKey, e->shiftKey, e->altKey, e->metaKey), 1);
            break;
        case EMSCRIPTEN_EVENT_MOUSEUP:
            if (pbEmccInternal.mouseInCanvas)
                pbCallCallback(MouseButton, e->button + 1, TranslateWebMod(e->ctrlKey, e->shiftKey, e->altKey, e->metaKey), 0);
            break;
        case EMSCRIPTEN_EVENT_MOUSEMOVE:
            if (pbEmccInternal.mouseInCanvas)
                pbCallCallback(MouseMove, e->clientX - (pbEmccInternal.screenW / 2) + (pbEmccInternal.canvasW / 2), e->clientY, e->movementX, e->movementY);
            break;
        case EMSCRIPTEN_EVENT_MOUSEENTER:
            pbEmccInternal.mouseInCanvas = 1;
            return 1;
        case EMSCRIPTEN_EVENT_MOUSELEAVE:
            pbEmccInternal.mouseInCanvas = 0;
            return 0;
        case EMSCRIPTEN_EVENT_CLICK:
        case EMSCRIPTEN_EVENT_DBLCLICK:
        default:
            return 0;
    }
    return 1;
}

static EM_BOOL wheel_callback(int type, const EmscriptenWheelEvent* e, void* user_data) {
    pbCallCallback(MouseScroll, e->deltaX, e->deltaY, TranslateWebMod(e->mouse.ctrlKey, e->mouse.shiftKey, e->mouse.altKey, e->mouse.metaKey));
    return 1;
}

static EM_BOOL uievent_callback(int type, const EmscriptenUiEvent* e, void* user_data) {
    pbEmccInternal.screenW = e->documentBodyClientWidth;
    pbEmccInternal.screenH = e->documentBodyClientHeight;
    emscripten_get_element_css_size(canvas, (double*)&pbEmccInternal.canvasW, (double*)&pbEmccInternal.canvasH);
    pbInternal.windowWidth = pbEmccInternal.canvasW;
    pbInternal.windowHeight = pbEmccInternal.canvasH;
    pbCallCallback(Resized, pbEmccInternal.screenW, pbEmccInternal.screenH);
    return 1;
}

static EM_BOOL focusevent_callback(int type, const EmscriptenFocusEvent* e, void* user_data) {
    pbCallCallback(Focus, type == EMSCRIPTEN_EVENT_FOCUS);
    return 1;
}

static const char* beforeunload_callback(int eventType, const void *reserved, void *userData) {
    return "Do you really want to leave the page?";
}

int pbBeginNative(int w, int h, const char *title, pbFlags flags) {
    emscripten_set_canvas_element_size(canvas, w, h);
    if (title)
        emscripten_set_window_title(title);
    pbInternal.windowWidth = w;
    pbInternal.windowHeight = h;

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

    return 1;
}

int pbPollNative(void) {
    // Nothing to do here
    return 1;
}

void pbFlushNative(pbImage *buffer) {
    if (!buffer || !buffer->buffer || !buffer->width || !buffer->height)
        return;
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
            data[i+0] = (val >> 16) & 0xFF;
            data[i+1] = (val >> 8) & 0xFF;
            data[i+2] = val & 0xFF;
            data[i+3] = 0xFF;
            src++;
            i += 4;
        }

        ctx.putImageData(img, 0, 0);
    }, buffer->width, buffer->height, buffer->buffer);
}

void pbEndNative(void) {
    // Nothing to do here
}

void pbSetWindowSizeNative(unsigned int w, unsigned int h) {
    pbInternal.windowWidth = pbEmccInternal.canvasW = w;
    pbInternal.windowHeight = pbEmccInternal.canvasH = h;
    emscripten_set_element_css_size(canvas, (double)w, (double)h);
}

void pbSetWindowTitleNative(const char *title) {
    emscripten_set_window_title(title);
}
