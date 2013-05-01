#ifndef CHUNK_HPP
#define CHUNK_HPP

#include "block.hpp"
#include "block_library.hpp"
#include "coordinate.hpp"
#include <cstdint>
#include <map>
#include <memory>
#include <set>

// NOTE: All coordinates are world coordinates, not relative to the chunk.
class Chunk
{
public:
	static const int SIZE = 1 << 4;		// Range of x and z dimensions
	static const int DEPTH = 1 << 7;	// Range of y dimension

	// Both x and z are in units of chunks
	Chunk(int x = 0, int z = 0, unsigned int seed = 0);

	int x() const { return m_x; }
	int z() const { return m_z; }
	const std::map<Coordinate, std::unique_ptr<Block>>& blocks() const { return m_blocks; }

	// This pointer will be invalidated if the block is removed
	const Block* get(const Coordinate& location) const;

	void newBlock(int x, int y, int z, BlockLibrary::Tag tag);
	void removeBlock(const Coordinate& location);

private:
	static const int SCALE = 1 << 5;	// Scale of top-level terrain features

	int m_x, m_z;
	std::map<Coordinate, std::unique_ptr<Block>> m_blocks;
};

#endif