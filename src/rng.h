/* rng.h -- https://github.com/takeiteasy/fwp
 
 fun-with-pixels is a hot-reloadable software-rendering library
 
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

#ifndef FWP_RNG_HEADER
#define FWP_RNG_HEADER
#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>

void rngInit(uint64_t initialSeed);
uint64_t rngRandom(void);
float rngRandomFloat(void);
int rngRandomIntRange(int min, int max);
float rngRandomFloatRange(float min, float max);

uint8_t* rngCellularAutomataMap(unsigned int width, unsigned int height, unsigned int fillChance, unsigned int smoothIterations, unsigned int survive, unsigned int starve);
uint8_t* rngPerlinNoiseMap(unsigned int width, unsigned int height, float z, float offsetX, float offsetY, float scale, float lacunarity, float gain, float octaves);
float rngPerlinNoise(float x, float y, float z);

#if defined(__cplusplus)
}
#endif
#endif // FWP_RNG_HEADER
