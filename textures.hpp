#ifndef TEXTURES_HPP
#define TEXTURES_HPP

#include <array>
#include <cstdint>
#include <GL/glew.h>

GLuint createCubeMap(const std::array<const char*, 6>& fileNames);

#endif