#ifndef SHADERS_HPP
#define SHADERS_HPP

#include <GL/glew.h>

GLuint loadShader(const char* fileName, GLenum shaderType);
GLuint linkShaders(GLuint vertexShader, GLuint fragmentShader);

#endif