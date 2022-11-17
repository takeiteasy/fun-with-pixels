# pp

_pp_ **[ˈpiːpiː]** or "_Pixel Playground_" is a small enviroment for quick experimentation with software rendering. Very simple API, build on top of my own [bitmap](https://github.com/takeiteasy/headers/blob/master/bitmap.h) library. See [here](https://takeiteasy.github.io/pp/) for documentation.

Includes a live coding solution, located in ```live``` directory (see [building](https://github.com/takeiteasy/pp#building) section for more info). Additionally, a [sokol_app](https://github.com/floooh/sokol/blob/master/sokol_app.h)-like application wrapper and more can be found in the ```extra``` directory.

**WIP** -- See [TODO](https://github.com/takeiteasy/pp#todo) section for current progress.

## Example

```c
#include "pp.h"

int main(int argc, const char *argv[]) {
    // Create window
    ppBegin(640, 480, "pp", ppResizable);
    // Create bitmap for framebuffer
    Bitmap pbo;
    InitBitmap(&pbo, 320, 240);
    // Loop until window is closed
    while (ppPoll()) {
        // Clear bitmap
        FillBitmap(&pbo, Black);
        // Draw bitmap to window
        ppFlush(&pbo);
    }
    // Clean up
    DestroyBitmap(&pbo);
    ppEnd();
    return 0;
}
```

## Building

The easiest way is to drop ```pp.c``` and ```pp.h``` into your project and add the appropriate flags (Mac: ```-framework Cocoa```, Windows: ```-lgdi32``` and Linux: ```-lX11```).

To build pp as a library, simple run ```make```. This will generate ```pp.c``` in the root directory and then create a shared library in the ```build``` folder. Generating pp.c requires Ruby to be installed, if you don't have Ruby run ```make library``` instead.

Also available is a small live coding enviroment, inspired by [this](https://nullprogram.com/blog/2014/12/23/). To get started, change to the ```live``` directory and run ```make all```. This will build the base application and the shared library. 

You can now run ```./pp -p libapp.dylib``` (.dylib on Mac, .so on Linux and .dll on Windows) to start the live program. Now simply edit ```app.c``` and run ```make library``` while the program is still running and the changes will be reloaded automatically.

On Windows, there is a simple batch file to build pp as a dll or build a program. Run ```build.bat``` with no arguments to build a dll file, or pass files as arguments to build an application, e.g. ```build.bat .\examples\basic.c```.

**NOTE**: The batch file uses the MSVC compiler, so Visual Studio or the [Visual Studio build tools](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022) must be installed. See [here](https://learn.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=msvc-170#developer_command_prompt_shortcuts) for more info.

## Gallery

Nothing to see yet

## TODO

- [X] ~~Documentation~~
- [X] ~~Mac context~~
- [ ] Windows context (WIP)
- [ ] Linux (X11) context
- [ ] WebAssembly (Emscripten) context
- [ ] Sixel context
- [ ] Live coding (~~Mac~~, Windows, Linux)
- [X] ~~Add bitmap.h to pp internally~~
- [ ] Fix bitmap API to fit with rest of the library
- [ ] Bindings (Ruby, Lua)

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
