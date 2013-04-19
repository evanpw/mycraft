#include "chunk.hpp"
#include "noise.hpp"
#include <cstdlib>

Chunk::Chunk()
{
	Noise stoneDepth(Chunk::BITS, Chunk::SIZE / 4);
	Noise dirtDepth(Chunk::BITS, 3.0);

	for (int i = 0; i < SIZE; ++i)
	{
		for (int j = 0; j < SIZE; ++j)
		{
			int stone = stoneDepth(i, j);
			int dirt = dirtDepth(i, j);
			int tree = (rand() % 256 == 0) ? (4 + rand() % 3) : 0;

			for (int k = 0; k < stone; ++k)
				newBlock(i, k, j, BlockLibrary::STONE);

			for (int k = stone; k < stone + dirt; ++k)
				newBlock(i, k, j, BlockLibrary::DIRT);

			for (int k = stone + dirt; k < stone + dirt + tree; ++k)
				newBlock(i, k, j, BlockLibrary::TREE);
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