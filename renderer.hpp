#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "block_library.hpp"
#include "camera.hpp"
#include "chunk.hpp"
#include "world.hpp"
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
	Renderer(int width, int height, World& world);
	~Renderer();

	Renderer(const Renderer& other) = delete;
	Renderer& operator=(const Renderer& other) = delete;

	void render(const Camera& camera);
	void setSize(int width, int height);

	// Tells the renderer that the given chunk has been modified, so that cached
	// information is no longer up-to-date
	void invalidate(const Chunk* chunk);

private:
	void findFrustum(const Camera& camera);

	// Generates the vertex buffer for the given chunk. This will only ever be
	// called from the chunkMaker thread
	void processChunk(const Chunk* chunk, ChunkRenderingData* data);

	// This is an accessor only. If the given data has not been generated, returns
	// nullptr
	ChunkRenderingData* getRenderingData(const Chunk* chunk);

	// Assumes that chunk and chunkData are not null
	void renderChunk(const Chunk* chunk, const ChunkRenderingData* chunkData);

	void buildViewProjectionMatrix(const Camera& camera) const;

	int m_width, m_height;
	World& m_world;

	std::set<std::pair<int, int>> m_queue;

	std::map<const Chunk*, std::unique_ptr<ChunkRenderingData>> m_chunkData;

	std::unique_ptr<BlockLibrary> m_blockLibrary;

	GLuint m_programId;

	// Shader input variables
	GLint m_position, m_texCoord, m_normal;

	// Shader uniform variables
	GLint m_modelMatrix, m_vpMatrix, m_highlight, m_textureSampler, m_resolution, m_sunPosition, m_brightness;
};

#endif