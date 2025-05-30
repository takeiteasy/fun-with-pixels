/* fwp.h -- https://github.com/takeiteasy/fun-with-pixels

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

#ifndef FWP_HEADER
#define FWP_HEADER
#if defined(__cplusplus)
extern "C" {
#endif

#include "pb.h"
#include "rng.h"

typedef struct fwpState fwpState;

typedef enum {
#define X(NAME, ARGS) NAME##Event,
    FWP_PB_CALLBACKS
#undef X
} pbEventType;

typedef struct {
    struct {
        int button;
        int isdown;
        struct {
            unsigned int x, y;
            float dx, dy;
        } position;
        struct {
            float dx, dy;
        } wheel;
    } mouse;
    struct {
        pbKey key;
        int isdown;
    } keyboard;
    pbMod modifier;
    struct {
        int focused, closed;
        struct {
            unsigned int width, height;
        } size;
    } window;
    pbEventType type;
} pbEvent;

typedef struct {
    int windowWidth;
    int windowHeight;
    const char *windowTitle;
    fwpState*(*init)(pbImage*);
    void(*deinit)(fwpState*);
    void(*reload)(fwpState*);
    void(*unload)(fwpState*);
    int(*event)(fwpState*, pbEvent*);
    int(*tick)(fwpState*, pbImage*, double);
} fwpScene;

#if defined(__cplusplus)
}
#endif
#endif // FWP_HEADER
