#include "chunk.hpp"
#include "cube.hpp"
#include "perlin_noise.hpp"
#include <cstdlib>
#include <ctime>
#include <glm/gtc/matrix_transform.hpp>

Chunk::Chunk(int x, int z, unsigned int seed)
: m_x(x), m_z(z)
{
	PerlinNoise heightMap(seed);
	PerlinNoise noise(seed + 1);
	PerlinNoise caves(seed + 2);

	// Lower means more mountains and valleys
	const float SMOOTHNESS = 25.0;

	// Larger means flatter
	const float DETAIL = 1 / 16.0;

	// Larger means more overhangs and caves
	const float CARVING = 2.0;

	// Larger means more caves
	const float CAVES = 3.0;

	for (int i = 0; i < SIZE; ++i)
	{
		for (int j = 0; j < SIZE; ++j)
		{
			float heightSample = heightMap.sample(SMOOTHNESS * (x * SIZE + i), 0.0, SMOOTHNESS * (z * SIZE + j));
			float height = (DEPTH / 2) + SCALE * heightSample; //(0.5 + 0.25 * heightSample);

			for (int k = 0; k < DEPTH; ++k)
			{
				//float sample = noise.sample(x + i / float(SIZE), k / float(DEPTH), z + j / float(SIZE));
				float sample = noise.sample(DETAIL * (x * SIZE + i), CARVING * DETAIL * k, DETAIL * (z * SIZE + j));
				sample += (height - k) / (SCALE / 4.0);

				float caveSample = caves.sample(DETAIL * (x * SIZE + i), CAVES * DETAIL * k, DETAIL * (z * SIZE + j));
				caveSample = pow(caveSample, 3.0);

				// Ground threshold
				if (sample > 0.0f && caveSample > -0.1)
				{
					// Stone threshold
					if (sample > 0.5f)
						newBlock(x * SIZE + i, k, z * SIZE + j, BlockLibrary::STONE);
					else
						newBlock(x * SIZE + i, k, z * SIZE + j, BlockLibrary::DIRT);
				}
			}
		}
	}

	// Insert water at the water level, and convert top-level dirt to
	// grass
	for (int i = 0; i < SIZE; ++i)
	{
		for (int j = 0; j < SIZE; ++j)
		{
			for (int k = DEPTH - 1; k >= 0; --k)
			{
				Coordinate location(x * SIZE + i, k, z * SIZE + j);

				if (!isEmpty(location))
				{
					auto& block = m_blocks[location];
					if (block->blockType == BlockLibrary::DIRT)
						block->blockType = BlockLibrary::GRASS;

					// We only work on the top-most block in a column.
					break;
				}
				else if (k < 0.45 * DEPTH)
				{
					while (isEmpty(location))
					{
						newBlock(location.x, location.y, location.z, BlockLibrary::WATER);
						--location.y;
					}

					break;
				}
			}
		}
	}
}

const Block* Chunk::get(const Coordinate& location) const
{
	auto i = m_blocks.find(location);
	if (i == m_blocks.end())
	{
		return nullptr;
	}
	else
	{
		return i->second.get();
	}
}

void Chunk::newBlock(int x, int y, int z, BlockLibrary::Tag tag)
{
	Coordinate location(x, y, z);
	m_blocks[location] = std::unique_ptr<Block>(new Block(location, tag));
}

void Chunk::removeBlock(const Coordinate& location)
{
	m_blocks.erase(location);
}

bool Chunk::isTransparent(const Coordinate& location) const
{
	const Block* block = get(location);
	return (block == nullptr || block->blockType == BlockLibrary::WATER);
}

bool Chunk::isSolid(const Coordinate& location) const
{
	const Block* block = get(location);
	return (block && block->blockType != BlockLibrary::WATER);
}

bool Chunk::isEmpty(const Coordinate& location) const
{
	const Block* block = get(location);
	return (block == nullptr);
}

bool Chunk::openToSky(const Coordinate& location) const
{
	Coordinate current = location.addY(1);
	while (current.y < DEPTH)
	{
		if (!isTransparent(current))
			return false;

		++current.y;
	}

	return true;
}

unsigned int Chunk::getLiveFaces(const Coordinate& r) const
{
	// TODO: Precompute a lot of this
	unsigned int mask = 0;
	if (isTransparent(r) && !isEmpty(r))
	{
		if (isEmpty(r.addX(1))) mask |= PLUS_X;
		if (isEmpty(r.addX(-1))) mask |= MINUS_X;
		if (isEmpty(r.addY(1))) mask |= PLUS_Y;
		if (isEmpty(r.addY(-1))) mask |= MINUS_Y;
		if (isEmpty(r.addZ(1))) mask |= PLUS_Z;
		if (isEmpty(r.addZ(-1))) mask |= MINUS_Z;
	}
	else
	{
		if (isTransparent(r.addX(1))) mask |= PLUS_X;
		if (isTransparent(r.addX(-1))) mask |= MINUS_X;
		if (isTransparent(r.addY(1))) mask |= PLUS_Y;
		if (isTransparent(r.addY(-1))) mask |= MINUS_Y;
		if (isTransparent(r.addZ(1))) mask |= PLUS_Z;
		if (isTransparent(r.addZ(-1))) mask |= MINUS_Z;
	}

	return mask;
}

std::vector<Vertex> Chunk::rebuildMesh()
{
	std::vector<Vertex> vertices;

	unsigned int masks[6] =
	{
		PLUS_X, MINUS_X,
		PLUS_Y, MINUS_Y,
		PLUS_Z, MINUS_Z
	};

	// First pass is for opaque blocks
	for (auto& itr : m_blocks)
	{
		const std::unique_ptr<Block>& block = itr.second;
		if (block->blockType == BlockLibrary::WATER)
			continue;

		// Translate the cube mesh to the appropriate place in world coordinates
		glm::mat4 model = glm::translate(glm::mat4(1.0f), block->location.vec3());

		unsigned int liveFaces = getLiveFaces(block->location);
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

					vertices.push_back(vertex);
				}
			}
		}
	}

	// Second pass is for transparent blocks
	for (auto& itr : m_blocks)
	{
		const std::unique_ptr<Block>& block = itr.second;
		if (block->blockType != BlockLibrary::WATER)
			continue;

		// Translate the cube mesh to the appropriate place in world coordinates
		glm::mat4 model = glm::translate(glm::mat4(1.0f), block->location.vec3());

		unsigned int liveFaces = getLiveFaces(block->location);
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

					vertices.push_back(vertex);
				}
			}
		}
	}

	//std::cout << "Vertex count: " << vertices.size() << std::endl;
	//std::cout << "VBO size: " << (sizeof(Vertex) * vertices.size() / (1 << 20)) << "MB" << std::endl;

	// Count on the RVO, or this would be very painful
	return vertices;
}