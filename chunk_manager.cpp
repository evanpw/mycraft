#include <algorithm>
#include <array>
#include <boost/timer/timer.hpp>
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include "chunk_manager.hpp"
#include "renderer.hpp"

ChunkManager::ChunkManager(int seed)
: m_seed(seed)
{
	// Create a bunch of vertex buffers initially, so that we don't
	// have to keep allocating and deleting
	m_vboPool.resize(MAX_OBJECTS);
	glGenBuffers(MAX_OBJECTS, &m_vboPool[0]);
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

Chunk* ChunkManager::getChunk(int x, int z)
{
	// This avoids duplicating code, but it is ugly. Scott Meyers says this is the best
	// way.
	return const_cast<Chunk*>(static_cast<const ChunkManager&>(*this).getChunk(x, z));
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

Mesh* ChunkManager::getOrCreateMesh(const Chunk* chunk)
{
	if (m_meshes.find(chunk) == m_meshes.end())
	{
		assert(!m_vboPool.empty());

		std::unique_ptr<Mesh> mesh(new Mesh);
		mesh->vertexBuffer = m_vboPool.back();
		m_vboPool.pop_back();

		m_meshes[chunk] = std::move(mesh);
	}

	return m_meshes[chunk].get();
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

std::vector<const Mesh*> ChunkManager::getVisibleMeshes(const Camera& camera)
{
	if (!m_chunkQueue.empty())
	{
		auto i = std::min_element(m_chunkQueue.begin(), m_chunkQueue.end(), DistanceToCamera(camera));
		std::pair<int, int> chunkCoord = *i;

		Chunk* chunk = getChunk(chunkCoord.first, chunkCoord.second);
		if (!chunk)
		{
			loadOrCreateChunk(chunkCoord.first, chunkCoord.second);
		}
		else
		{
			Mesh* mesh = getOrCreateMesh(chunk);

			std::vector<Vertex> vertices = chunk->rebuildMesh();
			glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
			mesh->vertexCount = vertices.size();

			m_chunkQueue.erase(i);
		}
	}

	std::vector<const Mesh*> meshes;

	int x = floor(camera.eye.x / (float)Chunk::SIZE);
	int z = floor(camera.eye.z / (float)Chunk::SIZE);
	for (int i = -RENDER_RADIUS; i <= RENDER_RADIUS; ++i)
	{
		for (int j = -RENDER_RADIUS; j <= RENDER_RADIUS; ++j)
		{
			const Chunk* chunk = getChunk(x + i, z + j);
			const Mesh* mesh = chunk ? getMesh(chunk) : nullptr;
			if (!mesh)
			{
				m_chunkQueue.insert(std::make_pair(x + i, z + j));
				continue;
			}

			// Render dirty chunks also - it's better to show a slightly out-of-date
			// image rather than a giant hole in the world.
			meshes.push_back(mesh);
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

void ChunkManager::removeBlock(const Coordinate& location)
{
	int x = floor(location.x / float(Chunk::SIZE));
	int z = floor(location.z / float(Chunk::SIZE));

	Chunk* chunk = getChunk(x, z);
	if (chunk)
	{
		chunk->removeBlock(location);
		m_chunkQueue.insert(std::make_pair(x, z));
	}
}

void ChunkManager::createBlock(const Coordinate& location, BlockLibrary::Tag tag)
{
	int x = floor(location.x / float(Chunk::SIZE));
	int z = floor(location.z / float(Chunk::SIZE));

	Chunk* chunk = getChunk(x, z);
	if (chunk)
	{
		chunk->newBlock(location.x, location.y, location.z, tag);
		m_chunkQueue.insert(std::make_pair(x, z));
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