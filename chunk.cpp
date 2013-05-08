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
			float height = (DEPTH / 2) + SCALE * heightSample; //(0.5 + 0.25 * heightSample);

			for (int k = 0; k < DEPTH; ++k)
			{
				float sample = noise.sample(DETAIL * (x * SIZE + i), CARVING * DETAIL * k, DETAIL * (z * SIZE + j));
				sample += (height - k) / (SCALE / 4.0);

				// Ground threshold
				if (sample > 0.0f)
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

	// Convert top-level dirt to grass
	for (int i = 0; i < SIZE; ++i)
	{
		for (int j = 0; j < SIZE; ++j)
		{
			// Fill any gap below sea level with water
			for (int k = 0; k < 0.45 * DEPTH; ++k)
			{
				Coordinate location(x * SIZE + i, k, z * SIZE + j);
				if (!get(location))
				{
					newBlock(location.x, location.y, location.z, BlockLibrary::WATER);
				}
			}
		}
	}

	// Cut out some caves
	for (int i = 0; i < SIZE; ++i)
	{
		for (int j = 0; j < SIZE; ++j)
		{
			for (int k = 0; k < DEPTH; ++k)
			{
				Coordinate location(x * SIZE + i, k, z * SIZE + j);
				if (m_blocks.find(location) == m_blocks.end())
					continue;

				float caveSample = caves.sample(DETAIL * (x * SIZE + i), CAVES * DETAIL * k, DETAIL * (z * SIZE + j));
				caveSample = pow(caveSample, 3.0);

				// Ground threshold
				if (caveSample <= -0.1)
				{
					removeBlock(location);
				}
			}

			// Convert top-level dirt to grass
			for (int k = DEPTH - 1; k >= 0; --k)
			{
				Coordinate location(x * SIZE + i, k, z * SIZE + j);

				if (get(location))
				{
					auto& block = m_blocks[location];
					if (block->blockType == BlockLibrary::DIRT)
						block->blockType = BlockLibrary::GRASS;

					// We only work on the top-most block in a column.
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