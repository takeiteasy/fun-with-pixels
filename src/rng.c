/* rng.c -- https://github.com/takeiteasy/fwp
 
 The MIT License (MIT)

 Copyright (c) 2024 George Watson

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
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "rng.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <time.h>
#include <limits.h>

pbRng rngSetSeed(uint64_t seed) {
    return seed ? seed : (uint64_t)time(NULL);
}

pbRng rngInit(uint64_t initialSeed) {
    return rngSetSeed(initialSeed);
}

uint64_t rngRandom(pbRng *rng) {
    uint64_t v = *rng;
    v = (uint64_t)(v * 6364136223846793005ULL) + 1;
    *rng = v;
    return v;
}

float rngRandomFloat(pbRng *rng) {
    return (float)rngRandom(rng) / (float)ULLONG_MAX;
}

#define __SWAP(a, b)  \
    do                \
    {                 \
        int temp = a; \
        a = b;        \
        b = temp;     \
    } while (0)

int rngRandomIntRange(pbRng *rng, int min, int max) {
    if (min > max)
        __SWAP(min, max);
    return (int)(rngRandomFloat(rng) * (max - min + 1) + min);
}

float rngRandomFloatRange(pbRng *rng, float min, float max) {
    if (min > max)
        __SWAP(min, max);
    return rngRandomFloat(rng) * (max - min) + min;
}

#define __MAX(a, b)  ((a) > (b) ? (a) : (b))
#define __CLAMP(v, low, high)  ((v) < (low) ? (low) : ((v) > (high) ? (high) : (v)))

uint8_t* rngCellularAutomataMap(pbRng *rng, unsigned int width, unsigned int height, unsigned int fillChance, unsigned int smoothIterations, unsigned int survive, unsigned int starve) {
    size_t sz = width * height * sizeof(int);
    uint8_t *result = malloc(sz);
    memset(result, 0, sz);
    // Randomly fill the grid
    fillChance = __CLAMP(fillChance, 1, 99);
    for (int x = 0; x < width; x++)
        for (int y = 0; y < height; y++)
            result[y * width + x] = rngRandom(rng) % 100 + 1 < fillChance;
    // Run cellular-automata on grid n times
    for (int i = 0; i < __MAX(smoothIterations, 1); i++)
        for (int x = 0; x < width; x++)
            for (int y = 0; y < height; y++) {
                // Count the cell's living neighbours
                int neighbours = 0;
                for (int nx = x - 1; nx <= x + 1; nx++)
                    for (int ny = y - 1; ny <= y + 1; ny++)
                        if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                            if ((nx != x || ny != y) && result[ny * width + nx] > 0)
                                neighbours++;
                        } else
                            neighbours++;
                // Update cell based on neighbour and surive/starve values
                if (neighbours > survive)
                    result[y * width + x] = 1;
                else if (neighbours < starve)
                    result[y * width + x] = 0;
            }
    return result;
}

#define __REMAP(v, from1, to1, from2, to2) (((v) - (from1)) / ((to1) - (from1)) * ((to2) - (from2)) + (from2))

uint8_t* rngPerlinNoiseMap(unsigned int width, unsigned int height, float z, float offsetX, float offsetY, float scale, float lacunarity, float gain, float octaves) {
    float min = FLT_MAX, max = FLT_MIN;
    float *grid = malloc(width * height * sizeof(float));
    // Loop through grid and apply noise transformation to each cell
    for (int x = 0; x < width; ++x)
        for (int y = 0; y < height; ++y) {
            float freq = 2.f,
            amp  = 1.f,
            tot  = 0.f,
            sum  = 0.f;
            for (int i = 0; i < octaves; ++i) {
                sum  += rngPerlinNoise(((offsetX + x) / scale) * freq, ((offsetY + y) / scale) * freq, z) * amp;
                tot  += amp;
                freq *= lacunarity;
                amp  *= gain;
            }
            // Keep track of min + max values for remapping the range later
            grid[y * width + x] = sum = (sum / tot);
            if (sum < min)
                min = sum;
            if (sum > max)
                max = sum;
        }
    // Convert float values to 0-255 range
    uint8_t *result = malloc(width * height * sizeof(uint8_t));
    for (int x = 0; x < width; x++)
        for (int y = 0; y < height; y++) {
            float height = 255.f - (255.f * __REMAP(grid[y * width + x], min, max, 0, 1.f));
            result[y * width + x] = (uint8_t)height;
        }
    // Free float grid, return uint8 grid
    free(grid);
    return result;
}

static const float grad3[][3] = {
    { 1, 1, 0 }, { -1, 1, 0 }, { 1, -1, 0 }, { -1, -1, 0 },
    { 1, 0, 1 }, { -1, 0, 1 }, { 1, 0, -1 }, { -1, 0, -1 },
    { 0, 1, 1 }, { 0, -1, 1 }, { 0, 1, -1 }, { 0, -1, -1 }
};

static const unsigned int perm[] = {
    182, 232, 51, 15, 55, 119, 7, 107, 230, 227, 6, 34, 216, 61, 183, 36,
    40, 134, 74, 45, 157, 78, 81, 114, 145, 9, 209, 189, 147, 58, 126, 0,
    240, 169, 228, 235, 67, 198, 72, 64, 88, 98, 129, 194, 99, 71, 30, 127,
    18, 150, 155, 179, 132, 62, 116, 200, 251, 178, 32, 140, 130, 139, 250, 26,
    151, 203, 106, 123, 53, 255, 75, 254, 86, 234, 223, 19, 199, 244, 241, 1,
    172, 70, 24, 97, 196, 10, 90, 246, 252, 68, 84, 161, 236, 205, 80, 91,
    233, 225, 164, 217, 239, 220, 20, 46, 204, 35, 31, 175, 154, 17, 133, 117,
    73, 224, 125, 65, 77, 173, 3, 2, 242, 221, 120, 218, 56, 190, 166, 11,
    138, 208, 231, 50, 135, 109, 213, 187, 152, 201, 47, 168, 185, 186, 167, 165,
    102, 153, 156, 49, 202, 69, 195, 92, 21, 229, 63, 104, 197, 136, 148, 94,
    171, 93, 59, 149, 23, 144, 160, 57, 76, 141, 96, 158, 163, 219, 237, 113,
    206, 181, 112, 111, 191, 137, 207, 215, 13, 83, 238, 249, 100, 131, 118, 243,
    162, 248, 43, 66, 226, 27, 211, 95, 214, 105, 108, 101, 170, 128, 210, 87,
    38, 44, 174, 188, 176, 39, 14, 143, 159, 16, 124, 222, 33, 247, 37, 245,
    8, 4, 22, 82, 110, 180, 184, 12, 25, 5, 193, 41, 85, 177, 192, 253,
    79, 29, 115, 103, 142, 146, 52, 48, 89, 54, 121, 212, 122, 60, 28, 42,
    
    182, 232, 51, 15, 55, 119, 7, 107, 230, 227, 6, 34, 216, 61, 183, 36,
    40, 134, 74, 45, 157, 78, 81, 114, 145, 9, 209, 189, 147, 58, 126, 0,
    240, 169, 228, 235, 67, 198, 72, 64, 88, 98, 129, 194, 99, 71, 30, 127,
    18, 150, 155, 179, 132, 62, 116, 200, 251, 178, 32, 140, 130, 139, 250, 26,
    151, 203, 106, 123, 53, 255, 75, 254, 86, 234, 223, 19, 199, 244, 241, 1,
    172, 70, 24, 97, 196, 10, 90, 246, 252, 68, 84, 161, 236, 205, 80, 91,
    233, 225, 164, 217, 239, 220, 20, 46, 204, 35, 31, 175, 154, 17, 133, 117,
    73, 224, 125, 65, 77, 173, 3, 2, 242, 221, 120, 218, 56, 190, 166, 11,
    138, 208, 231, 50, 135, 109, 213, 187, 152, 201, 47, 168, 185, 186, 167, 165,
    102, 153, 156, 49, 202, 69, 195, 92, 21, 229, 63, 104, 197, 136, 148, 94,
    171, 93, 59, 149, 23, 144, 160, 57, 76, 141, 96, 158, 163, 219, 237, 113,
    206, 181, 112, 111, 191, 137, 207, 215, 13, 83, 238, 249, 100, 131, 118, 243,
    162, 248, 43, 66, 226, 27, 211, 95, 214, 105, 108, 101, 170, 128, 210, 87,
    38, 44, 174, 188, 176, 39, 14, 143, 159, 16, 124, 222, 33, 247, 37, 245,
    8, 4, 22, 82, 110, 180, 184, 12, 25, 5, 193, 41, 85, 177, 192, 253,
    79, 29, 115, 103, 142, 146, 52, 48, 89, 54, 121, 212, 122, 60, 28, 42
};

static float dot3(const float a[], float x, float y, float z) {
    return a[0]*x + a[1]*y + a[2]*z;
}

static float lerp(float a, float b, float t) {
    return (1 - t) * a + t * b;
}

static float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

#define FASTFLOOR(x) (((x) >= 0) ? (int)(x) : (int)(x)-1)

float rngPerlinNoise(float x, float y, float z) {
    /* Find grid points */
    int gx = FASTFLOOR(x);
    int gy = FASTFLOOR(y);
    int gz = FASTFLOOR(z);
    /* Relative coords within grid cell */
    float rx = x - gx;
    float ry = y - gy;
    float rz = z - gz;
    /* Wrap cell coords */
    gx = gx & 255;
    gy = gy & 255;
    gz = gz & 255;
    /* Calculate gradient indices */
    unsigned int gi[8];
    for (int i = 0; i < 8; i++)
        gi[i] = perm[gx+((i>>2)&1)+perm[gy+((i>>1)&1)+perm[gz+(i&1)]]] % 12;
    /* Noise contribution from each corner */
    float n[8];
    for (int i = 0; i < 8; i++)
        n[i] = dot3(grad3[gi[i]], rx - ((i>>2)&1), ry - ((i>>1)&1), rz - (i&1));
    /* Fade curves */
    float u = fade(rx);
    float v = fade(ry);
    float w = fade(rz);
    /* Interpolate */
    float nx[4];
    for (int i = 0; i < 4; i++)
        nx[i] = lerp(n[i], n[4+i], u);
    float nxy[2];
    for (int i = 0; i < 2; i++)
        nxy[i] = lerp(nx[i], nx[2+i], v);
    return lerp(nxy[0], nxy[1], w);
}
