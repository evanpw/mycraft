#ifndef BLOCK_LIBRARY_HPP
#define BLOCK_LIBRARY_HPP

#include <GL/glew.h>
#include <memory>
#include <vector>

enum Face {SIDE = 0, TOP = 1, BOTTOM = 2};

struct BlockType
{
	GLuint texture;
};

class BlockLibrary
{
public:
	typedef size_t Tag;

	// Certain block types need to be referenced by name in the code. For example,
	// in terrain generation
	static const Tag TREE = 0;
	static const Tag STONE = 1;
	static const Tag DIRT = 2;

	BlockLibrary();

	const BlockType& get(Tag tag) const;
	size_t size() const { return m_blockTypes.size(); }

private:
	std::vector<std::unique_ptr<BlockType>> m_blockTypes;
};

#endif