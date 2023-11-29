# pp

_pp_ **[ˈpiːpiː]** or "_Pixel Playground_" is a small minimal cross-platform software-rendering window + context. Click [here](https://takeiteasy.github.io/pp/) for documentation.

Platforms:
* Windows
* MacOS
* Linux
* WebAssembly (via [emscripten](https://github.com/emscripten-core/emscripten))

## Example
```c
#include "pp"

// Window event callbacks
void onKeyboard(void *userdata, int key, int modifier, int isDown);
void onMouseButton(void *userdata, int button, int modifier, int isDown);
void onMouseMove(void *userdata, int x, int y, float dx, float dy);
void onMouseScroll(void *userdata, float dx, float dy, int modifier);
void onFocus(void *userdata, int isFocused);
void onResized(void *userdata, int w, int h);
void onClosed(void *userdata);

// Buffer to store image data
static int test[640 * 480];

int main(int argc, const char *argv[]) {
    // Initialize pp and create 640x480 window titled "pp"
    ppBegin(640, 480, "pp", ppResizable);
    // Set window event callbacks ...
    // ppKeyboardCallback(onKeyboard);
    
    // Create a simple xor pattern in the image buffer
    for (int x = 0; x < 640; x++)
        for (int y = 0; y < 480; y++)
            test[y * 640 + x] = x ^ y;
    
    // Keep window open and poll events (must poll every frame)
    while (ppPoll()) {
        // do something here ...
        
        // Flush the buffer to the window
        ppFlush(test, 640, 480);
    }
    
    // Always clean your pp
    ppEnd();
    return 0;
}

```

## Building

The Makefile has three options; ```default``` will build example/basic.c, ```library``` will build a dynamic library and ```web``` will also build example/basic.c, except compiled to wasm.

It's also very easy to use in your own project. Simply copy (or fork) ```pp[platform].c``` and ```pp.h``` from the ```src``` directory. If you are on windows you will need to link ```-lgdi32```. If you're on Linux you must link ```-lX11 -lm``` and on MacOS ```-framework Cocoa```.

## License
```
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
```
