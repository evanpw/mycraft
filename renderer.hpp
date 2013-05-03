#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "block_library.hpp"
#include "camera.hpp"
#include "chunk.hpp"
#include "mesh.hpp"
#include <gl/glew.h>
#include <map>
#include <memory>

class Renderer
{
public:
	Renderer(int width, int height);
	~Renderer();

	Renderer(const Renderer& other) = delete;
	Renderer& operator=(const Renderer& other) = delete;

	void renderMeshes(const Camera& camera, const std::vector<const Mesh*>& meshes);
	void tintScreen(const glm::vec3& color);

	void setSize(int width, int height);
	int width() const { return m_width; }
	int height() const { return m_height; }

private:
	void buildViewProjectionMatrix(const Camera& camera) const;

	int m_width, m_height;

	std::unique_ptr<BlockLibrary> m_blockLibrary;

	GLuint m_vertexArray;

	// Shader for rendering chunks of terrain
	struct
	{
		GLuint programId;

		// Shader input variables
		GLint position, texCoord, normal;

		// Shader uniform variables
		GLint modelMatrix, vpMatrix, highlight, textureSampler;
		GLint resolution, sunPosition, brightness;
	} m_chunkShader;

	// Shader program for tinting the screen (for example,
	// when underwater).
	struct
	{
		GLuint programId;

		// Input variables
		GLint position;

		// Uniform variables
		GLint color;

		// Stores the vertices of a quad which covers the entire screen.
		GLuint vbo;
	} m_tintShader;
};

#endif