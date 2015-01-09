#ifndef MESH_HPP
#define MESH_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

struct Vertex
{
	GLfloat position[3];
	GLfloat texCoord[3];
	GLfloat lighting;
};

struct Mesh
{
	GLuint vertexBuffer;
	size_t opaqueVertices, transparentVertices;
};

void copyVector(GLfloat* dest, const glm::vec3& source);

#endif
