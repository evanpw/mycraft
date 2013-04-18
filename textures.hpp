#ifndef TEXTURES_HPP
#define TEXTURES_HPP

#include <cstdint>
#include <GL/glew.h>

uint32_t* readPng(const char* fileName, int* outWidth, int* outHeight);
GLuint makeTexture(const char* filename);

#endif