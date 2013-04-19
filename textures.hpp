#ifndef TEXTURES_HPP
#define TEXTURES_HPP

#include <cstdint>
#include <GL/glew.h>

bool readPng(const char* fileName, uint32_t* data);
GLuint makeTextures(const char* fileNames[]);

#endif