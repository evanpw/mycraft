#include "block_library.hpp"
#include "textures.hpp"
#include <iostream>

BlockLibrary::BlockLibrary()
{
	// TODO: Load all of this info from a file
	const char* textureFiles[][3] =
	{
		{"textures/tree_side.png", "textures/tree_top_bottom.png", "textures/tree_top_bottom.png"},
		{"textures/stone.png", "textures/stone.png", "textures/stone.png"},
		{"textures/grass_side.png", "textures/grass_top.png", "textures/grass_bottom.png"}
	};

	for (const char** fileNames : textureFiles)
	{
		BlockType* blockType = new BlockType;
		blockType->texture = makeTextures(fileNames);

		std::unique_ptr<BlockType> ptr(blockType);
		m_blockTypes.push_back(std::move(ptr));
	}
}

const BlockType& BlockLibrary::get(Tag tag) const
{
	return *m_blockTypes[tag];
}