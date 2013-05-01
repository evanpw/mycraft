#ifndef MESH_HPP
#define MESH_HPP

#include <gl/glew.h>
#include <glm/glm.hpp>
#include <vector>

struct Vertex
{
	GLfloat position[3];
	GLfloat texCoord[4];
	GLfloat normal[3];
};

struct Mesh
{
	GLuint vertexBuffer;
	size_t vertexCount;
};

void copyVector(GLfloat* dest, const glm::vec3& source);

#endif