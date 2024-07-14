# Fun With Pixels

> [!WARNING]
> Work in progress, see [TODO](#todo) section. Everything works, but features need adding.

__Fun With Pixels__ (_fwp_) is a cross-platform hot-reloading software-rendering enviroment for C, designed for quick experiments and testing. Edit your C code, rebuild and see the changes in real-time!

If you would like to try it out, see the [setting up](#setting-up) section. Also included is a utility library _pb_ that can function independently from the _fwp_ program. If you're interested in that, see the [pb](#pb) section.

![Preview](/preview.gif)

## Usage

```
usage: fwp -p [path] [options]

  -w/--width     Window width [default: 640]
  -h/--height    Window height [default: 480]
  -t/--title     Window title [default: "fwp"]
  -r/--resizable Enable resizable window
  -a/--top       Enable window always on top
  -p/--path      Path the dynamic library [required]
  -u/--usage     Display this message
```

## Setting up

To achieve hot-reloading in C, all the code that is to be reloaded is built as a dynamic library. The symbols are loaded from the library and updated by the _fwp_ executable; which remains running.

To make it easy, the user's code is put into a `scene`. Below is a barebones example of how to setup a scene. Look at the example and read the comments.

```c
// Firstly include the fwp header, which contains everything we need
#include "fwp.h"
#include <stdlib.h> // malloc

// A fwpState should be defined in each scene. This structure can contain whatever variables and types you want, but it must be defined like this. Do not typedef the struct definition, as it is already typedef'd in fwp.h
struct fwpState {
    pbColor clearColor;
};

static fwpState* init(void) {
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
```

Now you have your scene file, you will have to build it as a dynamic library. Make sure to link the necessary files. Below is an example of building on MacOS, please see the Makefile to see how to build on your platform.

```
clang -shared -fpic [scene file].c src/pb_cocoa.c src/rng.c -framework Cocoa -o scene.dylib
```

Now you have your dynamic library, you can run:

```
fwp -p scene.dylib
```

Provided you haven't changed the example, the window should appear and display a red background. If you now rebuild the scene with the same command as before, the screen will change to blue! That's about it really.

## pb

If you're interested in using _pb_ as a standalone library, it's very easy. It will be a similar process to before.

_pb_ currently supports 4 different backends; Windows (Win32api), MacOS (Cocoa), \*nix (X11) and WASM (Emscripten). Each backend has a different system library or dependency that you will need to link. Feel free to request or submit a pull request for other backends.

```c
#include "pb.h"

int main(int argc, const char *argv[]) {
    // Initalize a new window titled "hello!"
    pbBegin(640, 480, "hello!", 0);
    // Create an image for the framebuffer
    pbImage *fb = pbImageNew(640, 480);
    // Loop while the window is open + poll events
    while (pbPoll()) {
        // Fill the image buffer red
        pbImageFill(fb, RGB(255, 0, 0));
        // Flush the image to the window's framebuffer
        pbFlush(fb);
    }
    // Clean up
    pbEnd();
    return 0;
}
```

And to build the executable:

```
clang -Ideps -Isrc [source file].c src/pb_cocoa.c -framework Cocoa -o [your executable]
```

## TODO

- [ ] Frame timing + limiting
- [ ] Documentation + some examples
- [ ] Image loading + saving (stb_image.h + stb_image_write.h + qoi.h)
- [ ] ANSI escape parser for text rendering
- [ ] GIF support (gif_load.h + msf_gif.h)
- [ ] TTF loading + rendering (stb_truetype.h)

## Dependencies

- [nothings/stb](https://github.com/nothings/stb) [MIT/UNLICENSE]
- [phoboslab/qoi](https://github.com/phoboslab/qoi) [MIT]
- [notnullnotvoid/msf_gif](https://github.com/notnullnotvoid/msf_gif) [MIT/Public Domain]
- [hidefromkgb/gif_load](https://github.com/hidefromkgb/gif_load/) [Public Domain]
- [dhepper/font8x8](https://github.com/dhepper/font8x8/blob/master/font8x8_basic.h) [Public Domain]
- [skandhurkat/Getopt-for-Visual-Studio](https://github.com/skandhurkat/Getopt-for-Visual-Studio) [GNU GPLv3]
- [dlfcn-win32/dlfcn-win32](https://github.com/dlfcn-win32/dlfcn-win32) [MIT]
- Inspired by this [blog post](https://nullprogram.com/blog/2014/12/23/) by [skeeto](https://github.com/skeeto/interactive-c-demo) [UNLICENSE]

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
