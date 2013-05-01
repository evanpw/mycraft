#ifndef MESH_HPP
#define MESH_HPP

#include <gl/glew.h>
#include <vector>

struct Vertex
{
	GLfloat position[3];
	GLfloat texCoord[4];
	GLfloat normal[3];
};

struct Mesh
{
	// The vertex data is required to be stored in this vector only until it has
	// been uploaded to the GPU
	std::vector<Vertex> vertices;

	GLuint vertexBuffer;

	// This is only guaranteed to have the correct value only when the mesh has been
	// uploaded to a VBO
	size_t vertexCount;

	// This mesh has been built, but needs to be uploaded to the VBO
	bool needsUploaded;
};

#endif