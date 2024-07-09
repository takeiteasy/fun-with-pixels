/* rng.h -- https://github.com/takeiteasy/fwp
 
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

#ifndef FWP_RNG_HEADER
#define FWP_RNG_HEADER
#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>

#if defined(_WIN32) || defined(_WIN64)
#define FWP_RNG_WINDOWS
#endif

#if defined(FWP_RNG_WINDOWS) && !defined(FWP_RNG_NO_EXPORT)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

typedef uint64_t rng_t;

rng_t rngInit(uint64_t initialSeed);
uint64_t rngRandom(rng_t *rng);
float rngRandomFloat(rng_t *rng);
int rngRandomIntRange(rng_t *rng, int min, int max);
float rngRandomFloatRange(rng_t *rng, float min, float max);

uint8_t* rngCellularAutomataMap(rng_t *rng, unsigned int width, unsigned int height, unsigned int fillChance, unsigned int smoothIterations, unsigned int survive, unsigned int starve);
uint8_t* rngPerlinNoiseMap(unsigned int width, unsigned int height, float z, float offsetX, float offsetY, float scale, float lacunarity, float gain, float octaves);
float rngPerlinNoise(float x, float y, float z);

#if defined(__cplusplus)
}
#endif
#endif // FWP_RNG_HEADER
