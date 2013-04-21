#include "block_library.hpp"
#include "textures.hpp"
#include <array>
#include <iostream>

BlockLibrary::BlockLibrary()
{
	// TODO: Load all of this info from a file
	std::array<const char*, 6> textureFiles[] =
	{
		{{"textures/tree_side.png", "textures/tree_side.png", "textures/tree_top_bottom.png", "textures/tree_top_bottom.png", "textures/tree_side.png", "textures/tree_side.png"}},
		{{"textures/stone.png", "textures/stone.png", "textures/stone.png", "textures/stone.png", "textures/stone.png", "textures/stone.png"}},
		{{"textures/dirt.png", "textures/dirt.png", "textures/dirt.png", "textures/dirt.png", "textures/dirt.png", "textures/dirt.png"}},
		{{"textures/grass_side.png", "textures/grass_side.png", "textures/grass_top.png", "textures/dirt.png", "textures/grass_side.png", "textures/grass_side.png"}}
	};

	for (auto fileNames : textureFiles)
	{
		BlockType* blockType = new BlockType;
		blockType->texture = createCubeMap(fileNames);

		std::unique_ptr<BlockType> ptr(blockType);
		m_blockTypes.push_back(std::move(ptr));
	}
}

const BlockType& BlockLibrary::get(Tag tag) const
{
	return *m_blockTypes[tag];
}