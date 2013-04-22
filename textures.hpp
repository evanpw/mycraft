#ifndef TEXTURES_HPP
#define TEXTURES_HPP

#include <array>
#include <cstdint>
#include <GL/glew.h>

GLuint createCubeMap(const std::array<const char*, 6>& fileNames);
uint32_t* readPng(const char* fileName, size_t& width, size_t& height);
bool readPng(const char* fileName, uint32_t* buffer, size_t size);

#endif