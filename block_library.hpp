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
	static const Tag GRASS = 0;
	static const Tag DIRT = 1;
	static const Tag STONE = 2;

	BlockLibrary();

	const BlockType& get(Tag tag) const;
	GLuint getTextureArray() const { return m_textureArray; }
	size_t size() const { return m_blockTypes.size(); }

	size_t textureResolution() const { return m_resolution; }
	size_t texturePixels() const { return m_resolution * m_resolution; }
	size_t textureBytes() const { return 4 * texturePixels(); }

private:
	void buildGrassTextures(const std::string& directory, uint32_t* result);

	GLuint m_textureArray;
	std::vector<std::unique_ptr<BlockType>> m_blockTypes;
	size_t m_resolution;
};

#endif