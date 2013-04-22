#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "block_library.hpp"
#include "camera.hpp"
#include "chunk.hpp"
#include "world.hpp"
#include <gl/glew.h>
#include <map>
#include <memory>

struct ChunkRenderingData
{
	GLuint vertexBuffer;
	size_t vertexCount;
	bool dirty;
};

class Renderer
{
public:
	Renderer(int width, int height, const World& world);

	Renderer(const Renderer& other) = delete;
	Renderer& operator=(const Renderer& other) = delete;

	void render(const Camera& camera);
	void setSize(int width, int height);

	// Tells the renderer that the given chunk has been modified, so that cached
	// information is no longer up-to-date
	void invalidate(const Chunk* chunk);

private:
	// Fills the vertex buffer specified by data.vertexBuffer with the current
	// data from chunk. Updates data.vertexCount and set data.dirty to false.
	void processChunk(const Chunk* chunk, ChunkRenderingData& data);

	const ChunkRenderingData& getRenderingData(const Chunk* chunk);
	void renderChunk(const Chunk* chunk);

	void buildViewProjectionMatrix(const Camera& camera) const;

	int m_width, m_height;
	const World& m_world;

	std::map<const Chunk*, ChunkRenderingData> m_chunkData;

	std::unique_ptr<BlockLibrary> m_blockLibrary;

	GLuint m_programId;

	// Shader input variables
	GLint m_position, m_texCoord, m_normal;

	// Shader uniform variables
	GLint m_modelMatrix, m_vpMatrix, m_highlight, m_textureSampler, m_resolution;
};

#endif