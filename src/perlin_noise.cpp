#include "perlin_noise.hpp"

#include <cmath>
#include <cstdint>
#include <utility>

PerlinNoise::PerlinNoise(unsigned int seed) {
    // Permute the integers 0-255 using the seed
    srand(seed);

    uint8_t permutation[256];
    for (size_t i = 0; i < 256; ++i) permutation[i] = i;

    for (size_t i = 0; i < 256; ++i) {
        int j = i + rand() % (256 - i);
        std::swap(permutation[i], permutation[j]);
    }

    // Stack together two copies of the permutation
    for (size_t i = 0; i < 256; ++i) {
        p[256 + i] = p[i] = permutation[i];
    }
}

float PerlinNoise::sample(float x, float y, float z) {
    // Find unit cube that contains the point
    uint8_t X = floor(x);
    uint8_t Y = floor(y);
    uint8_t Z = floor(z);

    // Find relative x, y, z of point in cube
    x -= floor(x);
    y -= floor(y);
    z -= floor(z);

    // Compute fade curves for each of x, y, z
    float u = fade(x), v = fade(y), w = fade(z);

    // Hash coordinates of the 8 cube corners
    int A = p[X] + Y, AA = p[A] + Z, AB = p[A + 1] + Z, B = p[X + 1] + Y, BA = p[B] + Z,
        BB = p[B + 1] + Z;

    // Blend results from the 8 corners of the cube
    float result =
        lerp(w,
             lerp(v, lerp(u, grad(p[AA], x, y, z), grad(p[BA], x - 1, y, z)),
                  lerp(u, grad(p[AB], x, y - 1, z), grad(p[BB], x - 1, y - 1, z))),
             lerp(v, lerp(u, grad(p[AA + 1], x, y, z - 1), grad(p[BA + 1], x - 1, y, z - 1)),
                  lerp(u, grad(p[AB + 1], x, y - 1, z - 1), grad(p[BB + 1], x - 1, y - 1, z - 1))));

    return result;
}

float PerlinNoise::fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }

float PerlinNoise::lerp(float t, float a, float b) { return a + t * (b - a); }

float PerlinNoise::grad(int hash, float x, float y, float z) {
    // Convert lower 4 bits of hash code into 12 gradient directions
    int h = hash & 0xF;
    float u = h < 8 ? x : y;

    float v;
    if (h < 4) {
        v = y;
    } else {
        if (h == 12 || h == 14) {
            v = x;
        } else {
            v = z;
        }
    }

    float result = 0;
    if ((h & 1) == 0)
        result += u;
    else
        result -= u;

    if ((h & 2) == 0)
        result += v;
    else
        result -= v;

    return result;
}