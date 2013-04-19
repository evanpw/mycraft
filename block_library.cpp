#include "block_library.hpp"
#include "textures.hpp"

BlockLibrary::BlockLibrary()
{
	// TODO: Load all of this info from a file
	const char* textureFiles[] = {"tree.png", "stone.png", "grass.png"};
	for (const char* filename : textureFiles)
	{
		GLuint texture = makeTexture(filename);
		BlockType* blockType = new BlockType(texture);
		std::unique_ptr<BlockType> ptr(blockType);

		m_blockTypes.push_back(std::move(ptr));
	}
}

const BlockType& BlockLibrary::get(Tag tag) const
{
	return *m_blockTypes[tag];
}