#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "block_library.hpp"
#include "camera.hpp"
#include "chunk.hpp"
#include "mesh.hpp"
#include <gl/glew.h>
#include <map>
#include <memory>
#include <boost/thread.hpp>
#include <tbb/concurrent_queue.h>

struct ChunkRenderingData
{
	GLuint vertexBuffer;
	size_t vertexCount;
	bool dirty;
};

class Renderer
{
public:
	static const int RENDER_RADIUS = 4;

	Renderer(int width, int height);
	~Renderer();

	Renderer(const Renderer& other) = delete;
	Renderer& operator=(const Renderer& other) = delete;

	void uploadMesh(Mesh* mesh);
	void renderMeshes(const Camera& camera, const std::vector<Mesh*>& meshes);

	void setSize(int width, int height);

private:
	void buildViewProjectionMatrix(const Camera& camera) const;

	int m_width, m_height;

	std::unique_ptr<BlockLibrary> m_blockLibrary;

	GLuint m_vertexArray;
	GLuint m_programId;

	// Shader input variables
	GLint m_position, m_texCoord, m_normal;

	// Shader uniform variables
	GLint m_modelMatrix, m_vpMatrix, m_highlight, m_textureSampler, m_resolution, m_sunPosition, m_brightness;
};

#endif