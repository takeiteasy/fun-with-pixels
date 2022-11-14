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
#define PP_INPUT_IMPLEMENTATION
#include "ppInput.h"

typedef struct {
    bool unlockFramerate;
    double targetFPS;
    int width;
    int height;
    const char *title;
    ppFlags flags;
    int fbWidth;
    int fbHeight;
    int clearColor;
    bool(*init)(void);
    void(*preFrame)(void);
    bool(*tick)(double);
    bool(*fixedTick)(double);
    void(*render)(double);
    void(*postFrame)(void);
    void(*deinit)(void);
} ppApp;

Bitmap* ppPbo(void);
#define PBO ppPbo()

#if defined(__cplusplus)
}
#endif
#endif // pp_app_h

#if defined(PP_APP_IMPLEMENTATION)
extern ppApp Main(int argc, const char *argv[]);

Bitmap* ppPbo(void) {
    static Bitmap pbo;
    return &pbo;
}

#if defined(macintosh) || defined(Macintosh) || (defined(__APPLE__) && defined(__MACH__))
#define PP_MAC
#elif defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__WINDOWS__)
#define PP_WINDOWS
#endif

#if defined(PP_MAC)
#include <mach/mach_time.h>
#elif defined(PP_WINDOWS)
#include <windows.h>
#else
#include <time.h>
#endif

static uint64_t timerFrequency(void) {
#if defined(PP_MAC)
    mach_timebase_info_data_t info;
    mach_timebase_info(&info);
    uint64_t frequency = info.denom;
    frequency *= 1000000000L;
    frequency /= info.numer;
    return frequency;
#elif defined(PP_WINDOWS)
    LARGE_INTEGER frequency;
    if (!QueryPerformanceFrequency(&frequency))
        return 1000L;
    return frequency.QuadPart;
#else
    return 1000000000L;
#endif
}

static uint64_t timerTicks(void) {
#if defined(PP_MAC)
    return mach_absolute_time();
#elif defined(PP_WINDOWS)
    LARGE_INTEGER counter;
    if (!QueryPerformanceCounter(&counter))
        return timeGetTime();
    return counter.QuadPart;
#else
    struct timespec now;
    clock_gettime(SDL_MONOTONIC_CLOCK, &now);
    uint64_t ticks = now.tv_sec;
    ticks *= 1000000000L;
    ticks += now.tv_nsec;
    return ticks;
#endif
}

int main(int argc, const char *argv[]) {
    ppApp app = Main(argc, argv);
    
    ppBegin(app.width, app.height, app.title ? app.title : "pp", app.flags);
    if (app.init)
        app.init();
    InitBitmap(PBO, app.fbWidth == 0 ? app.width : app.fbWidth, app.fbHeight == 0 ? app.height : app.fbHeight);
    ppInitInput();
    
    uint64_t timer_frequency = timerFrequency();
    double update_rate = app.targetFPS == 0 ? 60.0 : app.targetFPS;
    int update_multiplicity = 1;
    double fixed_deltatime = 1.0 / update_rate;
    int64_t desired_frametime = timer_frequency / update_rate;
    int64_t vsync_maxerror = timer_frequency * .0002;
    int64_t time_60hz = timer_frequency / 60;
    int64_t snap_frequencies[7] = {
      time_60hz,        //60fps
      time_60hz*2,      //30fps
      time_60hz*3,      //20fps
      time_60hz*4,      //15fps
      (time_60hz+1)/2,  //120fps
      (time_60hz+2)/3,  //180fps
      (time_60hz+3)/4,  //240fps
    };
    const int time_history_count = 4;
    int64_t time_averager[time_history_count] = {desired_frametime, desired_frametime, desired_frametime, desired_frametime};
    int resync = 1;
    int64_t prev_frame_time = timerTicks();
    int64_t frame_accumulator = 0;
    
    int clearColor = app.clearColor == 0 ? Black : app.clearColor;
    
    while (ppPoll()) {
        if (app.preFrame)
            app.preFrame();
        
        int64_t current_frame_time = timerTicks();
        int64_t delta_time = current_frame_time - prev_frame_time;
        prev_frame_time = current_frame_time;
        
        if (delta_time > desired_frametime * 8)
            delta_time = desired_frametime;
        if (delta_time < 0)
            delta_time = 0;
        
        for (int i = 0; i < 7; ++i)
            if (labs(delta_time - snap_frequencies[i]) < vsync_maxerror) {
                delta_time = snap_frequencies[i];
                break;
            }
        
        for (int i = 0; i < time_history_count - 1; ++i)
            time_averager[i] = time_averager[i + 1];
        time_averager[time_history_count - 1] = delta_time;
        delta_time = 0;
        for (int i = 0; i < time_history_count; ++i)
            delta_time += time_averager[i];
        delta_time /= time_history_count;
        
        if ((frame_accumulator += delta_time) > desired_frametime * 8)
            resync = 1;
        
        if (resync) {
            frame_accumulator = 0;
            delta_time = desired_frametime;
            resync = 0;
        }
        
        double render_time = 1.0;
        if (app.unlockFramerate) {
            int64_t consumedDeltaTime = delta_time;
            
            while (frame_accumulator >= desired_frametime) {
                if (app.fixedTick)
                    app.fixedTick(fixed_deltatime);
                if (consumedDeltaTime > desired_frametime) {
                    if (app.tick)
                        app.tick(fixed_deltatime);
                    consumedDeltaTime -= desired_frametime;
                }
                frame_accumulator -= desired_frametime;
            }
            
            if (app.tick)
                app.tick((double)consumedDeltaTime / timer_frequency);
            render_time = (double)frame_accumulator / desired_frametime;
        } else {
            while (frame_accumulator >= desired_frametime*update_multiplicity) {
                for (int i = 0; i < update_multiplicity; ++i) {
                    if (app.tick)
                        app.tick(fixed_deltatime);
                    if (app.fixedTick)
                        app.fixedTick(fixed_deltatime);
                    frame_accumulator -= desired_frametime;
                }
            }
        }
        
        FillBitmap(PBO, clearColor);
        if (app.render)
            app.render(render_time);
        if (app.postFrame)
            app.postFrame();
        ppFlush(PBO);
        ppUpdateInput();
    }
    
    if (app.deinit)
        app.deinit();
    DestroyBitmap(PBO);
    ppEnd();
    return 0;
}
#endif
