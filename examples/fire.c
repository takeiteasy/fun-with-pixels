// basic.c - A simple example of how to use pp
// For infomation on bitmaps, see: https://github.com/takeiteasy/bitmap
#include "pp.h"
#include <time.h>

#define WIDTH  640
#define HEIGHT 480
static Bitmap pbo;
static Bitmap fire;
static bool quit = false;

typedef struct {
    int p1, p2;
    unsigned int buffer[17];
} Random;

static unsigned int rotl(unsigned int n, unsigned int r) {
    return (n << r) | (n >> (32 - r));
}

static unsigned int InitRandom(Random *r, unsigned int s) {
    if (!s)
        s = (unsigned int)clock();

    for (unsigned int i = 0; i < 17; i++) {
        s = s * 2891336453 + 1;
        r->buffer[i] = s;
    }

    r->p1 = 0;
    r->p2 = 10;
    return s;
}

static unsigned int RandomBits(Random *r) {
    unsigned int result = r->buffer[r->p1] = rotl(r->buffer[r->p2], 13) + rotl(r->buffer[r->p1], 9);

    if (--r->p1 < 0)
        r->p1 = 16;
    if (--r->p2 < 0)
        r->p2 = 16;

    return result;
}

static float RandomFloat(Random *r) {
    union {
        float value;
        unsigned int word;
    } convert = {
        .word = (RandomBits(r) >> 9) | 0x3f800000};
    return convert.value - 1.0f;
}

static unsigned int RandomInt(Random *r, int max) {
    return RandomBits(r) % max;
}

static unsigned int RandomIntRange(Random *r, int min, int max) {
    return RandomBits(r) % (max + 1 - min) + min;
}

static void onKeyboard(void *userdata, ppKey key, ppMod modifier, bool isDown) {
    if (key == KEY_ESCAPE && isDown)
        quit = true;
}

static Random rng;

static float FIRE_INTENSITY = 1.f;
static int FIRE_WIDTH = 20;
static int FIRE_HEIGHT = 40;
static int FIRE_PALETTE[7] = {
        -65536,
        -6807033,
        -331755,
        -410869,
        -489207,
        -1946157056,
        805306368
    };

static void loop(void) {
    DrawCharacter(&fire, '*', FIRE_WIDTH / 2 - 4, FIRE_HEIGHT - 8, FIRE_PALETTE[0]);
    for (int i = 0; i < FIRE_WIDTH * FIRE_HEIGHT; i++) {
        int x = RandomIntRange(&rng, 0, FIRE_WIDTH - 1);
        int y = RandomIntRange(&rng, 0, FIRE_HEIGHT - 1);
        int c = PGet(&fire, x, y);
        if (rgbA(c) <= 0)
            continue;
        int pIdx = -1;
        for (int j = 0; j < 7; j++) {
            if (FIRE_PALETTE[j] == c) {
                pIdx = j;
                break;
            }
        }
        
        int r = RandomFloat(&rng);
        if (r <= FIRE_INTENSITY) {
            int nx = x + RandomInt(&rng, 3) - 1;
            PSet(&fire, nx, y - 1, c);
            PSet(&fire, x, y, pIdx < 0 || ++pIdx >= 7 ? RGBA(0, 0, 0, 0) : FIRE_PALETTE[pIdx]);
        }
    }
    
    FillBitmap(&pbo, Black);
    int mx = WIDTH / 2 - FIRE_WIDTH / 2;
    int my = HEIGHT / 2 - FIRE_HEIGHT / 2;
    PasteBitmap(&pbo, &fire, mx, my);
    
    ppFlush(&pbo);
}

int main(int argc, const char *argv[]) {
    InitRandom(&rng, (unsigned int)clock());
    
    ppBegin(WIDTH, HEIGHT, "pp", ppResizable);
    ppKeyboardCallback(onKeyboard);
    
    InitBitmap(&pbo, WIDTH, HEIGHT);
    InitBitmap(&fire, FIRE_WIDTH, FIRE_HEIGHT);
    FillBitmap(&fire, RGBA(0, 0, 0, 0));
    
    while (ppPoll() && !quit)
        loop();
    
    DestroyBitmap(&pbo);
    ppEnd();
    return 0;
}
