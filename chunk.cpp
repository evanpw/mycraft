#include "chunk.hpp"
#include "perlin_noise.hpp"
#include <cstdlib>
#include <ctime>

Chunk::Chunk()
{
	unsigned int seed = time(0);
	PerlinNoise heightMap(seed);
	PerlinNoise noise(seed + 1);

	// Higher means more mountains and valleys
	const float ROUGHNESS = 10.0;

	for (int i = 0; i < SIZE; ++i)
	{
		for (int j = 0; j < SIZE; ++j)
		{
			float heightSample = heightMap.sample(ROUGHNESS * i / float(SIZE), 0.0, ROUGHNESS * j / float(SIZE));
			float height = 16 + 8 * heightSample;

			for (int k = 0; k < 32; ++k)
			{
				float sample = noise.sample(i / float(SIZE), k / float(32), j / float(SIZE));
				sample += (height - k) / 16.0;

				// Ground threshold
				if (sample > 0.0f)
				{
					// Stone threshold
					if (sample > 0.2f)
						newBlock(i, k, j, BlockLibrary::STONE);
					else
						newBlock(i, k, j, BlockLibrary::DIRT);
				}
			}
		}
	}

	updateLiveBlocks();
}

const Block* Chunk::get(const Coordinate& location) const
{
	auto i = m_allBlocks.find(location);
	if (i == m_allBlocks.end())
	{
		return nullptr;
	}
	else
	{
		return i->second.get();
	}
}

bool Chunk::isTransparent(const Coordinate& location) const
{
	const Block* block = get(location);
	return (block == nullptr);
}

bool Chunk::isSolid(const Coordinate& location) const
{
	const Block* block = get(location);
	return (block != nullptr);
}

void Chunk::newBlock(int x, int y, int z, BlockLibrary::Tag tag)
{
	Coordinate location(x, y, z);
	m_allBlocks[location] = std::unique_ptr<Block>(new Block(location, tag));
}

void Chunk::removeBlock(const Coordinate& location)
{
	std::cout << "Deleting " << location << std::endl;

	const Block* block = m_allBlocks[location].get();
	m_liveBlocks[block->blockType].erase(block);
	m_allBlocks.erase(location);

	// Must check all adjacent blocks because they may now be alive
	Coordinate neighbors[] = {
		location.addX(1), location.addX(-1),
		location.addY(1), location.addY(-1),
		location.addZ(1), location.addZ(-1)
	};

	for (auto& r : neighbors)
	{
		const Block* neighbor = get(r);
		if (neighbor)
		{
			m_liveBlocks[neighbor->blockType].insert(neighbor);
			std::cout << "Now live: " << neighbor->location << std::endl;
		}
	}
}

void Chunk::updateLiveBlocks()
{
	std::cout << "Total blocks: " << m_allBlocks.size() << std::endl;

	size_t liveCount = 0;
	m_liveBlocks.clear();
	for (auto& i : m_allBlocks)
	{
		const Coordinate& r = i.first;
		const std::unique_ptr<Block>& block = i.second;

		// Check all sides of the cube
		if (isTransparent(r.addX(1)) || isTransparent(r.addX(-1)) ||
			isTransparent(r.addY(1)) || isTransparent(r.addY(-1)) ||
			isTransparent(r.addZ(1)) || isTransparent(r.addZ(-1)))
		{
			m_liveBlocks[block->blockType].insert(block.get());
			++liveCount;
		}
	}

	std::cout << "Live blocks: " << liveCount << std::endl;
}