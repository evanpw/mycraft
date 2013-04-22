#include "block_library.hpp"
#include "textures.hpp"
#include <array>
#include <iostream>

const size_t TEXTURE_SIZE = 256;

BlockLibrary::BlockLibrary()
{
	// TODO: Load all of this info from a file
	std::vector<const char*> textureFiles =
	{
		"textures/tree_side.png", "textures/tree_side.png", "textures/tree_top_bottom.png", "textures/tree_top_bottom.png", "textures/tree_side.png", "textures/tree_side.png",
		"textures/stone.png", "textures/stone.png", "textures/stone.png", "textures/stone.png", "textures/stone.png", "textures/stone.png",
		"textures/dirt.png", "textures/dirt.png", "textures/dirt.png", "textures/dirt.png", "textures/dirt.png", "textures/dirt.png",
		"textures/grass_side.png", "textures/grass_side.png", "textures/grass_top.png", "textures/dirt.png", "textures/grass_side.png", "textures/grass_side.png"
	};

	/*
	for (auto fileNames : textureFiles)
	{
		BlockType* blockType = new BlockType;
		blockType->texture = createCubeMap(fileNames);

		std::unique_ptr<BlockType> ptr(blockType);
		m_blockTypes.push_back(std::move(ptr));
	}
	*/

	uint32_t* data = new uint32_t[textureFiles.size() * TEXTURE_SIZE * TEXTURE_SIZE];
	size_t offset = 0;
	for (const char* fileName : textureFiles)
	{
		readPng(fileName, &data[offset], TEXTURE_SIZE);
		offset += (TEXTURE_SIZE * TEXTURE_SIZE);
	}

	// Upload the texture data en masse
    glGenTextures(1, &m_textureArray);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, m_textureArray);
    glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGBA8, TEXTURE_SIZE, TEXTURE_SIZE, textureFiles.size(), 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP_ARRAY);
}

const BlockType& BlockLibrary::get(Tag tag) const
{
	return *m_blockTypes[tag];
}