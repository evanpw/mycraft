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

// Keeps track of the properties and textures for the various block types
class BlockLibrary
{
public:
	typedef size_t Tag;

	// Certain block types need to be referenced by name in the code. For example,
	// in terrain generation
	static const Tag TREE = 0;
	static const Tag STONE = 1;
	static const Tag DIRT = 2;
	static const Tag GRASS = 3;
	static const Tag LEAVES = 4;
	static const Tag WATER = 5;

	BlockLibrary();

	const BlockType& get(Tag tag) const;
	GLuint getTextureArray() const { return m_textureArray; }
	size_t size() const { return m_blockTypes.size(); }

private:
	GLuint m_textureArray;
	std::vector<std::unique_ptr<BlockType>> m_blockTypes;
};

#endif