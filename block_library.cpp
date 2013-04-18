#include "block_library.hpp"
#include "textures.hpp"

BlockLibrary::BlockLibrary()
{
	// TODO: Load all of this info from a file
	const char* textureFiles[] = {"tree.png", "stone.png"};
	for (size_t i = 0; i < 2; ++i)
	{
		GLuint texture = makeTexture(textureFiles[i]);
		BlockType* blockType = new BlockType(texture);
		std::unique_ptr<BlockType> ptr(blockType);

		m_blockTypes.push_back(std::move(ptr));
	}
}

const BlockType& BlockLibrary::get(Tag tag) const
{
	return *m_blockTypes[tag];
}