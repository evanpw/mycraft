#ifndef CHUNK_MANAGER_HPP
#define CHUNK_MANAGER_HPP

#include <GL/glew.h>

#include <map>
#include <memory>
#include <set>
#include <vector>

#include "block.hpp"
#include "camera.hpp"
#include "chunk.hpp"
#include "coordinate.hpp"
#include "mesh.hpp"

class ChunkManager {
public:
    static const int RENDER_RADIUS = 4;

    ChunkManager(int seed);

    std::vector<const Mesh*> getVisibleMeshes(const Camera& camera);

    // Access the world
    const Block* getBlock(const Coordinate& location) const;
    bool isTransparent(const Coordinate& location) const;
    bool isSolid(const Coordinate& location) const;
    bool isEmpty(const Coordinate& location) const;

    // Modify the world
    void removeBlock(const Coordinate& location);
    void createBlock(const Coordinate& location, BlockLibrary::Tag tag);

private:
    // The seed for the PRNG used by the terrain generator
    int m_seed;

    // Return null if the chunk is not resident or has not been generated
    const Chunk* getChunk(int x, int z) const;
    Chunk* getChunk(int x, int z);
    const Chunk* getChunk(const Coordinate& location) const;
    Chunk* getChunk(const Coordinate& location);

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

    // Determine which of the faces (if any) of a given block are not adjacent
    // to an opaque block
    static const unsigned int PLUS_X = 1 << 0;
    static const unsigned int MINUS_X = 1 << 1;
    static const unsigned int PLUS_Y = 1 << 2;
    static const unsigned int MINUS_Y = 1 << 3;
    static const unsigned int PLUS_Z = 1 << 4;
    static const unsigned int MINUS_Z = 1 << 5;
    unsigned int getLiveFaces(const Coordinate& r) const;

    // Determine all triangles which could possibly be visible
    void rebuildMesh(const Chunk* chunk, Mesh* mesh);

    std::map<const Chunk*, std::unique_ptr<Mesh>> m_meshes;
};

#endif
