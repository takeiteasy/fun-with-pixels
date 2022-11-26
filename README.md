# pp

_pp_ **[ˈpiːpiː]** or "_Pixel Playground_" is a small enviroment for quick experimentation with software rendering. Very simple API, build on top of my own [bitmap](https://github.com/takeiteasy/headers/blob/master/bitmap.h) library. See [here](https://takeiteasy.github.io/pp/) for documentation. Includes a live coding option built in, see [building](https://github.com/takeiteasy/pp#building) for more info.


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

The easiest way is to drop ```pp.c``` and ```pp.h``` into your project and add the appropriate flags (Mac: ```-framework Cocoa```, Windows: ```-lgdi32``` and Linux: ```-lX11```). Or, run ```make library``` to build a dynamic library.

To build the live-reload app, run ```make all``` or ```make run```. This will build an executable and library in the ```build``` folder and then run the app. Edit ```examples/live.c```, and run ```make``` or ```make game```, and the program will automatically reload the changes. Inspired by [this](https://nullprogram.com/blog/2014/12/23/).

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
