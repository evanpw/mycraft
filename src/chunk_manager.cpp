#include <algorithm>
#include <array>
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "chunk_manager.hpp"
#include "cube.hpp"
#include "renderer.hpp"

ChunkManager::ChunkManager(int seed) : m_seed(seed) {
    // Create a bunch of vertex buffers initially, so that we don't
    // have to keep allocating and deleting
    m_vboPool.resize(MAX_OBJECTS);
    glGenBuffers(MAX_OBJECTS, &m_vboPool[0]);
}

void ChunkManager::freeMesh(const Chunk* chunk) {
    auto i = m_meshes.find(chunk);
    if (i != m_meshes.end()) {
        // Return the vertex buffer to the pool to be reused
        m_vboPool.push_back(i->second->vertexBuffer);
        m_meshes.erase(i);
    }
}

const Chunk* ChunkManager::getChunk(int x, int z) const {
    auto i = m_chunks.find(std::make_pair(x, z));
    if (i == m_chunks.end()) {
        return nullptr;
    } else {
        return i->second.get();
    }
}

Chunk* ChunkManager::getChunk(int x, int z) {
    // This avoids duplicating code, but it is ugly. Scott Meyers says this is the best
    // way.
    return const_cast<Chunk*>(static_cast<const ChunkManager&>(*this).getChunk(x, z));
}

const Chunk* ChunkManager::getChunk(const Coordinate& location) const {
    int x = floor(location.x / float(Chunk::SIZE));
    int z = floor(location.z / float(Chunk::SIZE));

    return getChunk(x, z);
}

Chunk* ChunkManager::getChunk(const Coordinate& location) {
    return const_cast<Chunk*>(static_cast<const ChunkManager&>(*this).getChunk(location));
}

Mesh* ChunkManager::getMesh(const Chunk* chunk) const {
    auto i = m_meshes.find(chunk);
    if (i == m_meshes.end()) {
        return nullptr;
    } else {
        return i->second.get();
    }
}

Mesh* ChunkManager::getOrCreateMesh(const Chunk* chunk) {
    if (m_meshes.find(chunk) == m_meshes.end()) {
        assert(!m_vboPool.empty());

        std::unique_ptr<Mesh> mesh(new Mesh);
        mesh->vertexBuffer = m_vboPool.back();
        m_vboPool.pop_back();

        m_meshes[chunk] = std::move(mesh);
    }

    return m_meshes[chunk].get();
}

void ChunkManager::loadOrCreateChunk(int x, int z) {
    // TODO: Load from a file
    std::unique_ptr<Chunk> newChunk(new Chunk(x, z, m_seed));
    m_chunks[std::make_pair(x, z)] = std::move(newChunk);
}

class DistanceToCamera {
public:
    DistanceToCamera(const Camera& camera) { m_camera = camera.eye.xz(); }

    static glm::vec2 chunkCenter(const std::pair<int, int>& location) {
        return float(Chunk::SIZE) * glm::vec2(location.first + 0.5, location.second + 0.5);
    }

    bool operator()(const std::pair<int, int>& lhs, const std::pair<int, int>& rhs) {
        return glm::distance(chunkCenter(lhs), m_camera) <
               glm::distance(chunkCenter(rhs), m_camera);
    }

private:
    glm::vec2 m_camera;
};

std::vector<const Mesh*> ChunkManager::getVisibleMeshes(const Camera& camera) {
    if (!m_chunkQueue.empty()) {
        auto i =
            std::min_element(m_chunkQueue.begin(), m_chunkQueue.end(), DistanceToCamera(camera));

        // This chunk and all of its neighbors need to be loaded in order to determine
        // the live faces and create the mesh
        int x = i->first, z = i->second;
        std::array<std::pair<int, int>, 5> chunkCoords = {
            {{x, z}, {x + 1, z}, {x - 1, z}, {x, z + 1}, {x, z - 1}}};

        bool loadedChunk = false;
        for (std::pair<int, int>& chunkCoord : chunkCoords) {
            Chunk* chunk = getChunk(chunkCoord.first, chunkCoord.second);
            if (!chunk) {
                loadOrCreateChunk(chunkCoord.first, chunkCoord.second);
                loadedChunk = true;
                break;
            }
        }

        if (!loadedChunk) {
            Chunk* chunk = getChunk(x, z);
            Mesh* mesh = getOrCreateMesh(chunk);

            rebuildMesh(chunk, mesh);
            m_chunkQueue.erase(i);
        }
    }

    std::vector<std::pair<int, int>> visibleChunks;

    int x = floor(camera.eye.x / (float)Chunk::SIZE);
    int z = floor(camera.eye.z / (float)Chunk::SIZE);
    for (int i = -RENDER_RADIUS; i <= RENDER_RADIUS; ++i) {
        for (int j = -RENDER_RADIUS; j <= RENDER_RADIUS; ++j) {
            const Chunk* chunk = getChunk(x + i, z + j);
            const Mesh* mesh = chunk ? getMesh(chunk) : nullptr;
            if (!mesh) {
                m_chunkQueue.insert(std::make_pair(x + i, z + j));
                continue;
            }

            visibleChunks.push_back(std::make_pair(x + i, z + j));
        }
    }

    // Sort the visible chunks from front to back, to avoid overdrawing and make
    // transparency work correctly
    sort(visibleChunks.begin(), visibleChunks.end(), DistanceToCamera(camera));

    std::vector<const Mesh*> meshes;
    for (std::pair<int, int>& chunkCoord : visibleChunks) {
        const Mesh* mesh = getMesh(getChunk(chunkCoord.first, chunkCoord.second));

        meshes.push_back(mesh);
    }

    // std::cout << "Loaded chunks: " << m_chunks.size() << ", loaded meshes = " << m_meshes.size()
    // << std::endl; std::cout << "In queue: " << m_chunkQueue.size() << std::endl;

    // Free/unload chunks and meshes that are far away from the camera
    glm::vec2 camera2d = camera.eye.xz();

    // We have to do this loop manually because elements are being deleted inside the loop
    auto j = m_chunks.begin();
    while (j != m_chunks.end()) {
        auto current = j++;
        const std::pair<int, int>& location = current->first;
        const Chunk* chunk = current->second.get();

        glm::vec2 chunkCenter = DistanceToCamera::chunkCenter(location);
        if (glm::distance(chunkCenter, camera2d) > 5 * RENDER_RADIUS * Chunk::SIZE) {
            freeMesh(chunk);
            m_chunks.erase(current);
        } else if (getMesh(chunk) &&
                   glm::distance(chunkCenter, camera2d) > 2 * RENDER_RADIUS * Chunk::SIZE) {
            freeMesh(chunk);
        }
    }

    return meshes;
}

const Block* ChunkManager::getBlock(const Coordinate& location) const {
    const Chunk* chunk = getChunk(location);
    if (chunk) {
        return chunk->get(location);
    } else {
        return nullptr;
    }
}

void ChunkManager::removeBlock(const Coordinate& location) {
    Chunk* chunk = getChunk(location);
    if (chunk) {
        chunk->removeBlock(location);
        m_chunkQueue.insert(std::make_pair(chunk->x(), chunk->z()));
    }

    // If we remove a block adjacent to another chunk, its mesh also needs
    // to be built again
    std::array<Coordinate, 4> neighbors = {
        {location.addX(1), location.addX(-1), location.addZ(1), location.addZ(-1)}};

    for (Coordinate& neighbor : neighbors) {
        Chunk* otherChunk = getChunk(neighbor);
        if (otherChunk && otherChunk != chunk)
            m_chunkQueue.insert(std::make_pair(otherChunk->x(), otherChunk->z()));
    }
}

void ChunkManager::createBlock(const Coordinate& location, BlockLibrary::Tag tag) {
    Chunk* chunk = getChunk(location);
    if (chunk) {
        chunk->newBlock(location.x, location.y, location.z, tag);
        m_chunkQueue.insert(std::make_pair(chunk->x(), chunk->z()));
    }
}

bool ChunkManager::isTransparent(const Coordinate& location) const {
    const Chunk* chunk = getChunk(location);
    return (!chunk || chunk->isTransparent(location));
}

bool ChunkManager::isSolid(const Coordinate& location) const {
    const Chunk* chunk = getChunk(location);
    return (chunk && chunk->isSolid(location));
}

bool ChunkManager::isEmpty(const Coordinate& location) const {
    const Block* block = getBlock(location);
    return (block == nullptr);
}

unsigned int ChunkManager::getLiveFaces(const Coordinate& r) const {
    // TODO: Precompute a lot of this
    unsigned int mask = 0;
    if (isTransparent(r) && !isEmpty(r)) {
        if (isEmpty(r.addX(1))) mask |= PLUS_X;
        if (isEmpty(r.addX(-1))) mask |= MINUS_X;
        if (isEmpty(r.addY(1))) mask |= PLUS_Y;
        if (isEmpty(r.addY(-1))) mask |= MINUS_Y;
        if (isEmpty(r.addZ(1))) mask |= PLUS_Z;
        if (isEmpty(r.addZ(-1))) mask |= MINUS_Z;
    } else {
        if (isTransparent(r.addX(1))) mask |= PLUS_X;
        if (isTransparent(r.addX(-1))) mask |= MINUS_X;
        if (isTransparent(r.addY(1))) mask |= PLUS_Y;
        if (isTransparent(r.addY(-1))) mask |= MINUS_Y;
        if (isTransparent(r.addZ(1))) mask |= PLUS_Z;
        if (isTransparent(r.addZ(-1))) mask |= MINUS_Z;
    }

    return mask;
}

void ChunkManager::rebuildMesh(const Chunk* chunk, Mesh* mesh) {
    std::vector<Vertex> vertices;

    unsigned int masks[6] = {PLUS_X, MINUS_X, PLUS_Y, MINUS_Y, PLUS_Z, MINUS_Z};

    // Determine lighting for each face
    float lighting[6];
    for (size_t face = 0; face < 6; ++face) {
        glm::vec3 normal = glm::normalize(cubeMesh[face * 6].normal);
        glm::vec3 sun = glm::normalize(glm::vec3(-4.0, 2.0, 1.0));

        float diffuse = glm::clamp(std::abs(0.7 * glm::dot(normal, sun)), 0.0, 1.0);
        float ambient = 0.3;
        lighting[face] = glm::clamp(diffuse + ambient, 0.0f, 1.0f);
    }

    // First pass is for opaque blocks
    mesh->opaqueVertices = 0;
    for (auto& itr : chunk->blocks()) {
        const std::unique_ptr<Block>& block = itr.second;
        if (block->blockType == BlockLibrary::WATER) continue;

        // Translate the cube mesh to the appropriate place in world coordinates
        glm::mat4 model = glm::translate(glm::mat4(1.0f), block->location.vec3());

        unsigned int liveFaces = getLiveFaces(block->location);
        for (size_t face = 0; face < 6; ++face) {
            if (liveFaces & masks[face]) {
                for (size_t i = 0; i < 6; ++i) {
                    CubeVertex cubeVertex = cubeMesh[face * 6 + i];

                    Vertex vertex;
                    copyVector(vertex.position,
                               glm::vec3(model * glm::vec4(cubeVertex.position, 1.0)));
                    copyVector(vertex.texCoord,
                               glm::vec3(cubeVertex.texCoord, block->blockType * 6 + face));
                    vertex.lighting = lighting[face];

                    vertices.push_back(vertex);
                    ++mesh->opaqueVertices;
                }
            }
        }
    }

    // Second pass is for transparent blocks
    mesh->transparentVertices = 0;
    for (auto& itr : chunk->blocks()) {
        const std::unique_ptr<Block>& block = itr.second;
        if (block->blockType != BlockLibrary::WATER) continue;

        // Translate the cube mesh to the appropriate place in world coordinates
        glm::mat4 model = glm::translate(glm::mat4(1.0f), block->location.vec3());

        unsigned int liveFaces = getLiveFaces(block->location);
        for (size_t face = 0; face < 6; ++face) {
            if (liveFaces & masks[face]) {
                for (size_t i = 0; i < 6; ++i) {
                    CubeVertex cubeVertex = cubeMesh[face * 6 + i];

                    Vertex vertex;
                    copyVector(vertex.position,
                               glm::vec3(model * glm::vec4(cubeVertex.position, 1.0)));
                    copyVector(vertex.texCoord,
                               glm::vec3(cubeVertex.texCoord, block->blockType * 6 + face));
                    vertex.lighting = lighting[face];

                    vertices.push_back(vertex);
                    ++mesh->transparentVertices;
                }
            }
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

    // std::cout << "Vertex count: " << vertices.size() << std::endl;
    // std::cout << "VBO size: " << (sizeof(Vertex) * vertices.size() / (1 << 20)) << "MB" <<
    // std::endl;
}
