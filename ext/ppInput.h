/* ppInput.h -- https://github.com/takeiteasy/pp
 
 Simple input manager to automatically handle event callbacks
 
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
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef pp_input_h
#define pp_input_h
#if defined(__cplusplus)
extern "C" {
#endif

#include "pp.h"
#include <stdarg.h>

void ppInitInput(void);
void ppUpdateInput(void);

void ppInputKeyboard(void *userdata, ppKey key, ppMod modifier, bool isDown);
void ppInputMouseButton(void *userdata, int button, ppMod modifier, bool isDown);
void ppInputMouseMove(void *userdata, int x, int y, float dx, float dy);
void ppInputMouseScroll(void *userdata, float dx, float dy, ppMod modifier);
void ppInputFocus(void *userdata, bool isFocused);
void ppInputResized(void *userdata, int w, int h);
void ppInputClosed(void *userdata);
bool ppIsKeyDown(uint8_t key);
bool ppIsKeyUp(uint8_t key);
bool ppWasKeyPressed(uint8_t key);
bool ppAreKeysDown(int n, ...);
bool ppAnyKeysDown(int n, ...);
bool ppIsButtonDown(int button);
bool ppIsButtonUp(int button);
bool ppWasButtonPressed(int button);
void ppScroll(float *x, float *y);
void ppMousePosition(int *x, int *y);
void ppMouseDelta(int *x, int *y);
bool ppModifier(uint32_t modifier);

#if defined(__cplusplus)
}
#endif
#endif // pp_input_h

#if defined(PP_INPUT_IMPLEMENTATION)
static struct ppInputManager {
    struct {
        bool buttons[8];
        struct {
            int x, y;
        } Position;
        struct {
            float x, y;
        } Scroll;
    } Mouse, MousePrev;
    struct {
        bool keys[KEY_TICK];
    } Keyboard, KeyboardPrev;
    ppMod modifier, modifierPrev;
    struct {
        bool focused, closed;
        struct {
            int width, height;
        } Size;
    } Window, WindowPrev;
} ppInput;

void ppInputKeyboard(void *userdata, ppKey key, ppMod modifier, bool isDown) {
    ppInput.Keyboard.keys[(int)key] = isDown;
    ppInput.modifier = modifier;
}

void ppInputMouseButton(void *userdata, int button, ppMod modifier, bool isDown) {
    ppInput.Mouse.buttons[button - 1] = isDown;
    ppInput.modifier = modifier;
}

void ppInputMouseMove(void *userdata, int x, int y, float dx, float dy) {
    ppInput.Mouse.Position.x = x;
    ppInput.Mouse.Position.y = y;
}

void ppInputMouseScroll(void *userdata, float dx, float dy, ppMod modifier) {
    ppInput.Mouse.Scroll.x = dx;
    ppInput.Mouse.Scroll.y = dy;
    ppInput.modifier = modifier;
}

void ppInputFocus(void *userdata, bool isFocused) {
    ppInput.Window.focused = isFocused;
}

void ppInputResized(void *userdata, int w, int h) {
    ppInput.Window.Size.width  = w;
    ppInput.Window.Size.height = h;
}

void ppInputClosed(void *userdata) {
    ppInput.Window.closed = true;
}

void ppInitInput(void) {
    memset(&ppInput.Keyboard, 0, sizeof(ppInput.Keyboard));
    memset(&ppInput.KeyboardPrev, 0, sizeof(ppInput.Keyboard));
    memset(&ppInput.Mouse, 0, sizeof(ppInput.Mouse));
    memset(&ppInput.MousePrev, 0, sizeof(ppInput.Mouse));
    memset(&ppInput.Window, 0, sizeof(ppInput.Window));
    
#define X(NAME, ARGS) ppInput##NAME,
    ppCallbacks(PP_CALLBACKS NULL);
#undef X
}

void ppUpdateInput(void) {
    memcpy(&ppInput.KeyboardPrev, &ppInput.Keyboard, sizeof(ppInput.Keyboard));
    memcpy(&ppInput.MousePrev, &ppInput.Mouse, sizeof(ppInput.Mouse));
    memset(&ppInput.Mouse.Scroll, 0, sizeof(ppInput.Mouse.Scroll));
    memcpy(&ppInput.WindowPrev, &ppInput.Window, sizeof(ppInput.Window));
}

bool ppIsKeyDown(uint8_t key) {
    return ppInput.Keyboard.keys[key];
}

bool ppIsKeyUp(uint8_t key) {
    return !ppInput.Keyboard.keys[key];
}

bool ppWasKeyPressed(uint8_t key) {
    return ppInput.KeyboardPrev.keys[key] && !ppInput.Keyboard.keys[key];
}

bool ppAreKeysDown(int n, ...) {
    va_list keys;
    va_start(keys, n);
    bool ret = true;
    for (int i = 0, k = va_arg(keys, int); i < n; ++i, k = va_arg(keys, int))
        if (!ppInput.Keyboard.keys[k]) {
            ret = false;
            break;
        }
    va_end(keys);
    return ret;
}

bool ppAnyKeysDown(int n, ...) {
    va_list keys;
    va_start(keys, n);
    bool ret = false;
    for (int i = 0, k = va_arg(keys, int); i < n; ++i, k = va_arg(keys, int))
        if (ppInput.Keyboard.keys[k]) {
            ret = true;
            break;
        }
    va_end(keys);
    return ret;
}

bool ppIsButtonDown(int button) {
    return ppInput.Mouse.buttons[button - 1];
}

bool ppIsButtonUp(int button) {
    return !ppInput.Mouse.buttons[button - 1];
}

bool ppWasButtonPressed(int button) {
    return ppInput.MousePrev.buttons[button - 1] && !ppInput.Mouse.buttons[button - 1];
}

void ppScroll(float *x, float *y) {
    if (x)
        *x = ppInput.Mouse.Scroll.x;
    if (y)
        *y = ppInput.Mouse.Scroll.y;
}

void ppMousePosition(int *x, int *y) {
    if (x)
        *x = ppInput.Mouse.Position.x;
    if (y)
        *y = ppInput.Mouse.Position.y;
}

void ppMouseDelta(int *x, int *y) {
    if (x)
        *x = ppInput.Mouse.Position.x - ppInput.MousePrev.Position.x;
    if (y)
        *y = ppInput.Mouse.Position.y - ppInput.MousePrev.Position.y;
}

bool ppModifier(uint32_t modifier) {
    return ppInput.modifier == modifier;
}
#endif
