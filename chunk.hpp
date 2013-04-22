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
	static const int SIZE = 1 << 5;		// Size of chunk x and z dimensions
	static const int DEPTH = 1 << 7;	// Maximum y coordinates
	static const int SCALE = 1 << 5;	// Scale of top-level terrain features

	// x and z are in units of SIZE blocks
	Chunk(int x = 0, int z = 0, unsigned int seed = 0);

	const std::map<Coordinate, std::unique_ptr<Block>>& allBlocks() const { return m_allBlocks; }

	const Block* get(const Coordinate& location) const;
	void removeBlock(const Coordinate& location);

private:
	// Does not update liveBlocks, only allBlocks
	void newBlock(int x, int y, int z, BlockLibrary::Tag tag);

	std::map<Coordinate, std::unique_ptr<Block>> m_allBlocks;
};

#endif