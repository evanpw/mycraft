#include "chunk.hpp"
#include "perlin_noise.hpp"
#include <cstdlib>
#include <ctime>

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
			float height = SCALE * (0.5 + 0.25 * heightSample);

			for (int k = 0; k < DEPTH; ++k)
			{
				//float sample = noise.sample(x + i / float(SIZE), k / float(DEPTH), z + j / float(SIZE));
				float sample = noise.sample(DETAIL * (x * SIZE + i), CARVING * DETAIL * k, DETAIL * (z * SIZE + j));
				sample += (height - k) / (SCALE / 4.0);

				float caveSample = caves.sample(DETAIL * (x * SIZE + i), CAVES * DETAIL * k, DETAIL * (z * SIZE + j));

				// Ground threshold
				if (sample > 0.0f && caveSample > -0.5)
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

	// Dirt with air above it becomes grass
	for (auto& i : m_blocks)
	{
		const Coordinate& location = i.first;
		auto& block = i.second;

		if (block->blockType == BlockLibrary::DIRT && !get(location.addY(1)))
			block->blockType = BlockLibrary::GRASS;
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