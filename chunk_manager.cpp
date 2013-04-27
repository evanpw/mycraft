#include <algorithm>
#include <array>
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "chunk_manager.hpp"
#include "renderer.hpp"

void copyVector(GLfloat* dest, const glm::vec3& source)
{
	memcpy(dest, glm::value_ptr(source), 3 * sizeof(GLfloat));
}

struct CubeVertex
{
	glm::vec3 position, normal;
};

const std::array<CubeVertex, 36> cubeMesh =
{{
	// Right face
	{glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)},
	{glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f)},
	{glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)},

	{glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f)},
	{glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)},
	{glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f)},


	// Left face
	{glm::vec3(0.0f, 1.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f)},
	{glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f)},
	{glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f)},

	{glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f)},
	{glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f)},
	{glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f)},


	// Top face
	{glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)},
	{glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)},
	{glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f)},

	{glm::vec3(0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f)},
	{glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f)},
	{glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)},


	// Bottom face
	{glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)},
	{glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)},
	{glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)},

	{glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)},
	{glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)},
	{glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)},


	// Front face
	{glm::vec3(0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)},
	{glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)},
	{glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)},

	{glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)},
	{glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)},
	{glm::vec3(0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)},


	// Back face
	{glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)},
	{glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)},
	{glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)},

	{glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)},
	{glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)},
	{glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)},
}};

ChunkManager::ChunkManager(int seed)
: m_seed(seed)
{
	// Create a bunch of vertex buffers initially, so that we don't
	// have to keep allocating and deleting
	m_vboPool.resize(MAX_OBJECTS);
	glGenBuffers(MAX_OBJECTS, &m_vboPool[0]);
}

void ChunkManager::createMesh(const Chunk* chunk)
{
	assert(!m_vboPool.empty());

	Mesh* mesh = new Mesh;
	mesh->vertexBuffer = m_vboPool.back();
	m_vboPool.pop_back();
	mesh->needsRebuilt = true;

	m_meshes[chunk] = std::unique_ptr<Mesh>(mesh);
}

void ChunkManager::freeMesh(const Chunk* chunk)
{
	auto i = m_meshes.find(chunk);
	if (i != m_meshes.end())
	{
		// Return the vertex buffer to the pool to be reused
		m_vboPool.push_back(i->second->vertexBuffer);
		m_meshes.erase(i);
	}
}

const Chunk* ChunkManager::getChunk(int x, int z) const
{
	auto i = m_chunks.find(std::make_pair(x, z));
	if (i == m_chunks.end())
	{
		return nullptr;
	}
	else
	{
		return i->second.get();
	}
}

Mesh* ChunkManager::getMesh(const Chunk* chunk) const
{
	auto i = m_meshes.find(chunk);
	if (i == m_meshes.end())
	{
		return nullptr;
	}
	else
	{
		return i->second.get();
	}
}

void ChunkManager::loadOrCreateChunk(int x, int z)
{
	// TODO: Load from a file
	std::unique_ptr<Chunk> newChunk(new Chunk(x, z, m_seed));
	m_chunks[std::make_pair(x, z)] = std::move(newChunk);
}

class DistanceToCamera
{
public:
	DistanceToCamera(const Camera& camera)
	{
		m_camera = camera.eye.xz;
	}

	static glm::vec2 chunkCenter(const std::pair<int, int>& location)
	{
		return float(Chunk::SIZE) * glm::vec2(location.first + 0.5, location.second + 0.5);
	}

	bool operator()(const std::pair<int, int>& lhs, const std::pair<int, int>& rhs)
	{
		return glm::distance(chunkCenter(lhs), m_camera) < glm::distance(chunkCenter(rhs), m_camera);
	}

private:
	glm::vec2 m_camera;
};

const size_t MAX_PER_FRAME = 1;
std::vector<Mesh*> ChunkManager::getVisibleMeshes(const Camera& camera)
{
	size_t processed = 0;
	while (!m_chunkQueue.empty() && processed < MAX_PER_FRAME)
	{
		auto i = std::min_element(m_chunkQueue.begin(), m_chunkQueue.end(), DistanceToCamera(camera));
		std::pair<int, int> chunkCoord = *i;
		m_chunkQueue.erase(i);

		if (!getChunk(chunkCoord.first, chunkCoord.second))
		{
			loadOrCreateChunk(chunkCoord.first, chunkCoord.second);
			++processed;
			if (processed >= MAX_PER_FRAME) break;
		}

		const Chunk* chunk = getChunk(chunkCoord.first, chunkCoord.second);
		if (!getMesh(chunk))
		{
			createMesh(chunk);
		}

		if (getMesh(chunk)->needsRebuilt)
		{
			rebuildMesh(chunk);
			++processed;
		}
	}

	std::vector<Mesh*> meshes;

	int x = floor(camera.eye.x / (float)Chunk::SIZE);
	int z = floor(camera.eye.z / (float)Chunk::SIZE);
	for (int i = -RENDER_RADIUS; i <= RENDER_RADIUS; ++i)
	{
		for (int j = -RENDER_RADIUS; j <= RENDER_RADIUS; ++j)
		{
			const Chunk* chunk = getChunk(x + i, z + j);
			if (!chunk)
			{
				m_chunkQueue.insert(std::make_pair(x + i, z + j));
				continue;
			}

			Mesh* mesh = getMesh(chunk);
			if (!mesh)
			{
				m_chunkQueue.insert(std::make_pair(x + i, z + j));
			}
			else
			{
				if (mesh->needsRebuilt)
				{
					m_chunkQueue.insert(std::make_pair(x + i, z + j));
				}

				// Render dirty chunks also - it's better to show a slightly out-of-date
				// image rather than a giant hole in the world.
				meshes.push_back(mesh);
			}
		}
	}

	//std::cout << "Loaded chunks: " << m_chunks.size() << ", loaded meshes = " << m_meshes.size() << std::endl;
	//std::cout << "In queue: " << m_chunkQueue.size() << std::endl;

	// Free/unload chunks and meshes that are far away from the camera
	glm::vec2 camera2d = camera.eye.xz;

	// We have to do this loop manually because elements are being deleted inside the loop
	auto j = m_chunks.begin();
	while (j != m_chunks.end())
	{
		auto current = j++;
		const std::pair<int, int>& location = current->first;
		const Chunk* chunk = current->second.get();

		glm::vec2 chunkCenter = DistanceToCamera::chunkCenter(location);
		if (glm::distance(chunkCenter, camera2d) > 5 * RENDER_RADIUS * Chunk::SIZE)
		{
			freeMesh(chunk);
			m_chunks.erase(current);
		}
		else if (getMesh(chunk) && glm::distance(chunkCenter, camera2d) > 2 * RENDER_RADIUS * Chunk::SIZE)
		{
			freeMesh(chunk);
		}
	}

	return meshes;
}

const Block* ChunkManager::getBlock(const Coordinate& location) const
{
	int x = floor(location.x / float(Chunk::SIZE));
	int z = floor(location.z / float(Chunk::SIZE));

	const Chunk* chunk = getChunk(x, z);
	if (chunk)
	{
		return chunk->get(location);
	}
	else
	{
		return nullptr;
	}
}

bool ChunkManager::isTransparent(const Coordinate& location) const
{
	const Block* block = getBlock(location);

	// TODO: Support transparent blocks other than air
	return (block == nullptr);
}

bool ChunkManager::isSolid(const Coordinate& location) const
{
	const Block* block = getBlock(location);

	// TODO: Support non-solid blocks other than air
	return (block != nullptr);
}

unsigned int ChunkManager::getLiveFaces(const Coordinate& r) const
{
	// TODO: Precompute a lot of this
	unsigned int mask = 0;
	if (isTransparent(r.addX(1))) mask |= PLUS_X;
	if (isTransparent(r.addX(-1))) mask |= MINUS_X;
	if (isTransparent(r.addY(1))) mask |= PLUS_Y;
	if (isTransparent(r.addY(-1))) mask |= MINUS_Y;
	if (isTransparent(r.addZ(1))) mask |= PLUS_Z;
	if (isTransparent(r.addZ(-1))) mask |= MINUS_Z;

	return mask;
}

void ChunkManager::rebuildMesh(const Chunk* chunk)
{
	Mesh* mesh = getMesh(chunk);
	assert(mesh != nullptr);

	mesh->vertices.clear();

	unsigned int masks[6] =
	{
		PLUS_X, MINUS_X,
		PLUS_Y, MINUS_Y,
		PLUS_Z, MINUS_Z
	};

	// Create the vertex data
	for (auto& itr : chunk->blocks())
	{
		const std::unique_ptr<Block>& block = itr.second;

		// Translate the cube mesh to the appropriate place in world coordinates
		glm::mat4 model = glm::translate(glm::mat4(1.0f), block->location.vec3());

		unsigned int liveFaces = getLiveFaces(block->location);
		if (liveFaces == 0) continue;
		for (size_t face = 0; face < 6; ++face)
		{
			if (liveFaces & masks[face])
			{
				for (size_t i = 0; i < 6; ++i)
				{
					CubeVertex cubeVertex = cubeMesh[face * 6 + i];

					Vertex vertex;
					copyVector(vertex.position, glm::vec3(model * glm::vec4(cubeVertex.position, 1.0)));
					copyVector(vertex.texCoord, cubeVertex.position);
					vertex.texCoord[3] = block->blockType;
					copyVector(vertex.normal, cubeVertex.normal);

					mesh->vertices.push_back(vertex);
				}
			}
		}
	}

	// std::cout << "Vertex count: " << mesh->vertices.size() << std::endl;
	// std::cout << "VBO size: " << (sizeof(Vertex) * mesh->vertices.size() / (1 << 20)) << "MB" << std::endl;

	mesh->needsRebuilt = false;
	mesh->needsUploaded = true;
}