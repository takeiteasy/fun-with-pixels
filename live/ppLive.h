#ifndef LIVE_APP_H
#define LIVE_APP_H
#include "pp.h"

typedef struct State State;

typedef struct {
    State*(*init)(void);
    void(*deinit)(State*);
    void(*reload)(State*);
    void(*unload)(State*);
    bool(*tick)(State*, Bitmap*, double);
} App;

extern const App pp;
#endif // LIVE_APP_H
