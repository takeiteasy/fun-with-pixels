/* ppApp.h -- https://github.com/takeiteasy/pp
 
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

#ifndef pp_app_h
#define pp_app_h
#if defined(__cplusplus)
extern "C" {
#endif

#include "pp.h"
#include "ppInput.h"

typedef struct {
    int width;
    int height;
    const char *title;
    ppFlags flags;
    int fbWidth;
    int fbHeight;
    int clearColor;
    bool(*init)(void);
    bool(*tick)(Bitmap*, double);
    void(*deinit)(void);
} ppApp;

#if defined(__cplusplus)
}
#endif
#endif // pp_app_h

#if defined(PP_APP_IMPLEMENTATION)
extern ppApp Main(int argc, const char *argv[]);

int main(int argc, const char *argv[]) {
    ppApp app = Main(argc, argv);
    if (!app.width || !app.height || !app.tick)
        return EXIT_FAILURE;
    
    ppBegin(app.width, app.height, app.title ? app.title : "pp", app.flags);
    if (app.init)
        if (!app.init())
            return EXIT_FAILURE;
    Bitmap pbo;
    InitBitmap(&pbo, app.fbWidth == 0 ? app.width : app.fbWidth, app.fbHeight == 0 ? app.height : app.fbHeight);
    ppInitInput();
    
    int clearColor = app.clearColor == 0 ? Black : app.clearColor;
    double lastTime = ppTime();
    while (ppPoll()) {
        double now = ppTime();
        double delta = now - lastTime;
        lastTime = now;
        
        FillBitmap(&pbo, clearColor);
        if (!app.tick(&pbo, delta))
            break;
        ppFlush(&pbo);
        ppUpdateInput();
    }
    
    if (app.deinit)
        app.deinit();
    DestroyBitmap(&pbo);
    ppEnd();
    return EXIT_SUCCESS;
}
#endif
