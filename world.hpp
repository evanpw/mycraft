#ifndef WORLD_HPP
#define WORLD_HPP

#include "block.hpp"
#include "chunk.hpp"
#include "coordinate.hpp"
#include <map>
#include <memory>
#include <set>

// TODO: Make this completely thread safe, or at least specify what is safe
class World
{
public:
	World();

	const Chunk* getChunk(int x, int z) const;
	const Chunk* getChunk(const Coordinate& location) const;
	const Block* get(const Coordinate& location) const;

	bool isTransparent(const Coordinate& location) const;
	bool isSolid(int x, int y, int z) const { return isSolid(Coordinate(x, y, z)); } // This is a hack to support the awful collision-detection
	bool isSolid(const Coordinate& location) const;

	// Get the chunk at the given location, returning null if it doesn't exist.
	Chunk* getChunk(int x, int z);
	Chunk* getChunk(const Coordinate& location);

	// Get the chunk at the given location, creating it if ncessary.
	Chunk* chunkAt(int x, int z);

	// Get all blocks which are open on at least one side in the chunk with the given coordinates
	const std::set<const Block*>& liveBlocks(const Chunk* chunk) const;

	// Returns the chunk that the block was removed from
	const Chunk* removeBlock(const Coordinate& location);

private:
	const int m_seed;

	std::map<std::pair<int, int>, std::unique_ptr<Chunk>> m_chunks;

	// This is a cache which is generated lazily, so it's okay for const methods to modify it
	mutable std::map<const Chunk*, std::set<const Block*>> m_liveBlocks;
};

#endif