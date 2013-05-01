#ifndef CHUNK_MANAGER_HPP
#define CHUNK_MANAGER_HPP

#include "block.hpp"
#include "camera.hpp"
#include "chunk.hpp"
#include "coordinate.hpp"
#include "mesh.hpp"
#include <set>
#include <map>
#include <memory>
#include <vector>
#include <gl/glew.h>

class ChunkManager
{
public:
	static const int RENDER_RADIUS = 8;

	ChunkManager(int seed);

	std::vector<const Mesh*> getVisibleMeshes(const Camera& camera);

	// Access the world
	const Block* getBlock(const Coordinate& location) const;
	bool isTransparent(const Coordinate& location) const;
	bool isSolid(const Coordinate& location) const;

	// Modify the world
	void removeBlock(const Coordinate& location);

private:
	// The seed for the PRNG used by the terrain generator
	int m_seed;

	// Return null if the chunk is not resident or has not been generated
	const Chunk* getChunk(int x, int z) const;
	Chunk* getChunk(int x, int z);

	// The first of these will return nullptr if the mesh has not been
	// generated yet.
	Mesh* getMesh(const Chunk* chunk) const;
	Mesh* getOrCreateMesh(const Chunk* chunk);

	static const size_t MAX_OBJECTS = 10 * RENDER_RADIUS * RENDER_RADIUS;
	std::vector<GLuint> m_vboPool;
	void freeMesh(const Chunk* chunk);

	std::set<std::pair<int, int>> m_chunkQueue;
	void loadOrCreateChunk(int x, int z);
	std::map<std::pair<int, int>, std::unique_ptr<Chunk>> m_chunks;

	std::map<const Chunk*, std::unique_ptr<Mesh>> m_meshes;
};

#endif