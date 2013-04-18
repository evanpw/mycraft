#ifndef CHUNK_HPP
#define CHUNK_HPP

#include "block.hpp"
#include "block_library.hpp"
#include "coordinate.hpp"
#include <cstdint>
#include <map>
#include <memory>
#include <set>

class Chunk
{
public:
	static const uint8_t BITS = 8;
	static const int SIZE = 1 << BITS;

	Chunk(const std::vector<std::vector<int>>& heightMap);

	const std::map<BlockLibrary::Tag, std::set<const Block*>>& liveBlocks() const { return m_liveBlocks; }

	const Block* get(const Coordinate& location) const;

	bool isTransparent(const Coordinate& location) const;
	bool isSolid(int x, int y, int z) const { return isSolid(Coordinate(x, y, z)); } // This is a hack to support the awful collision-detection
	bool isSolid(const Coordinate& location) const;

	void removeBlock(const Coordinate& location);

private:
	// Does not update liveBlocks, only allBlocks
	void newBlock(int x, int y, int z, BlockLibrary::Tag tag);

	// Determines the live blocks from scratch
	void updateLiveBlocks();

	std::map<Coordinate, std::unique_ptr<Block>> m_allBlocks;
	std::map<BlockLibrary::Tag, std::set<const Block*>> m_liveBlocks;
};

#endif