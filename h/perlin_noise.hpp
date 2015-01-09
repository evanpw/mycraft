#ifndef PERLIN_NOISE
#define PERLIN_NOISE

#include <glm/gtc/noise.hpp>

// Adapted from http://mrl.nyu.edu/~perlin/noise/
class PerlinNoise
{
public:
	PerlinNoise(unsigned int seed = 0);
	float sample(float x, float y, float z);

private:
	float fade(float t);
	float lerp(float t, float a, float b);
	float grad(int hash, float x, float y, float z);

	uint8_t p[512];
};

#endif