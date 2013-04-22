#include "world.hpp"

const Chunk* World::getChunk(int x, int z) const
{
	auto i = chunks.find(std::make_pair(x, z));
	return i->second.get();
}

Chunk* World::getChunk(int x, int z)
{
	return chunks[std::make_pair(x, z)].get();
}

const Chunk* World::getChunk(const Coordinate& location) const
{
	return getChunk(floor(location.x / (float)Chunk::SIZE), floor(location.z / (float)Chunk::SIZE));
}

Chunk* World::getChunk(const Coordinate& location)
{
	return getChunk(floor(location.x / (float)Chunk::SIZE), floor(location.z / (float)Chunk::SIZE));
}

Chunk* World::chunkAt(int x, int z)
{
	Chunk* chunk = getChunk(x, z);
	if (!chunk)
	{
		std::cout << "Creating chunk at " << x << ", " << z << std::endl;

		chunk = new Chunk(x, z);
		std::unique_ptr<Chunk> ptr(chunk);
		chunks[std::make_pair(x, z)] = std::move(ptr);

		// TODO: Update live blocks based on this, so that we can add a new
		// chunk in the middle of a game.
	}

	return chunk;
}

const Block* World::get(const Coordinate& location) const
{
	const Chunk* chunk = getChunk(location);
	if (chunk)
	{
		return chunk->get(location);
	}
	else
	{
		return nullptr;
	}
}

bool World::isTransparent(const Coordinate& location) const
{
	const Block* block = get(location);
	return (block == nullptr);
}

bool World::isSolid(const Coordinate& location) const
{
	const Block* block = get(location);
	return (block != nullptr);
}

const std::set<const Block*>& World::liveBlocks(const Chunk* chunk) const
{
	// If the live blocks have never been generated, then create them from scratch
	if (m_liveBlocks.find(chunk) == m_liveBlocks.end())
	{
		std::cout << "Total blocks: " << chunk->allBlocks().size() << ", ";

		size_t liveCount = 0;
		std::set<const Block*>& liveBlocks = m_liveBlocks[chunk];
		for (auto& i : chunk->allBlocks())
		{
			const Coordinate& r = i.first;
			const std::unique_ptr<Block>& block = i.second;

			// Check all sides of the cube
			if (isTransparent(r.addX(1)) || isTransparent(r.addX(-1)) ||
				isTransparent(r.addY(1)) || isTransparent(r.addY(-1)) ||
				isTransparent(r.addZ(1)) || isTransparent(r.addZ(-1)))
			{
				liveBlocks.insert(block.get());
				++liveCount;
			}
		}

		std::cout << "Live blocks: " << liveCount << std::endl;
	}

	return m_liveBlocks[chunk];
}

const Chunk* World::removeBlock(const Coordinate& location)
{
	Chunk* chunk = getChunk(location);
	const Block* block = get(location);
	m_liveBlocks[chunk].erase(block);

	// Must check all adjacent blocks because they may now be alive
	Coordinate neighbors[] = {
		location.addX(1), location.addX(-1),
		location.addY(1), location.addY(-1),
		location.addZ(1), location.addZ(-1)
	};

	for (auto& r : neighbors)
	{
		if (!isTransparent(r))
		{
			const Chunk* neighborChunk = getChunk(r);
			const Block* neighbor = get(r);
			m_liveBlocks[neighborChunk].insert(neighbor);
		}
	}

	chunk->removeBlock(location);
	return chunk;
}