#include "block_library.hpp"
#include "textures.hpp"
#include <array>
#include <cstring>
#include <iostream>
#include <glm/glm.hpp>

BlockLibrary::BlockLibrary()
{
	//std::string directory = "Misa500";
	std::string directory = "Sphax PureBDcraft 256x MC15";

	PngFile png(directory  + "/textures/blocks/" + std::string("dirt.png"));
	m_resolution = png.width();
	assert(png.width() == png.height());

	std::vector<std::string> textureFiles =
	{
		"dirt.png", "dirt.png", "dirt.png", "dirt.png", "dirt.png", "dirt.png",
		"stone.png", "stone.png", "stone.png", "stone.png", "stone.png", "stone.png",
	};

	uint32_t* data = new uint32_t[(textureFiles.size() + 6 * 2) * texturePixels()];

	buildGrassTextures(directory, &data[0]);
	buildWaterTextures(directory, &data[6 * texturePixels()]);
	size_t offset = 6 * 2;
	for (std::string& fileName : textureFiles)
	{
		std::string fullName = directory + "/textures/blocks/" + fileName;

		PngFile texture(fullName);
		assert(m_resolution == texture.width() && m_resolution == texture.height());
		texture.copyTo(&data[offset * texturePixels()]);

		++offset;
	}

	// Upload the texture data en masse
    glGenTextures(1, &m_textureArray);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, m_textureArray);
    glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGBA8, m_resolution, m_resolution, textureFiles.size() + 6 * 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP_ARRAY);
}

void BlockLibrary::buildGrassTextures(const std::string& directory, uint32_t* result)
{
	std::string prefix = directory + "/textures/blocks/";

	PngFile topTexture(prefix + "grass_top.png");
	assert(topTexture.width() == m_resolution && topTexture.height() == m_resolution);

	PngFile bottomTexture(prefix + "dirt.png");
	assert(bottomTexture.width() == m_resolution && bottomTexture.height() == m_resolution);

	PngFile sideTexture(prefix + "grass_side.png");
	assert(sideTexture.width() == m_resolution && sideTexture.height() == m_resolution);

	PngFile overlayTexture(prefix + "grass_side_overlay.png");
	assert(overlayTexture.width() == m_resolution && overlayTexture.height() == m_resolution);

	// All of the grass will be tinted with this color
	glm::vec3 grassGreen(0.57, 0.9, 0.30);

	topTexture.tint(grassGreen);
	overlayTexture.tint(grassGreen);
	sideTexture.overlayWith(overlayTexture);

	// Copy into the final locations
	sideTexture.copyTo(&result[0 * texturePixels()]);
	sideTexture.copyTo(&result[1 * texturePixels()]);
	topTexture.copyTo(&result[2 * texturePixels()]);
	bottomTexture.copyTo(&result[3 * texturePixels()]);
	sideTexture.copyTo(&result[4 * texturePixels()]);
	sideTexture.copyTo(&result[5 * texturePixels()]);
}

void BlockLibrary::buildWaterTextures(const std::string& directory, uint32_t* result)
{
	std::string prefix = directory + "/textures/blocks/";

	PngFile texture(prefix + "water.png");
	assert(texture.width() == m_resolution && texture.height() >= m_resolution);

	texture.cropHeight(m_resolution);

	for (size_t i = 0; i < 6; ++i)
		texture.copyTo(&result[i * texturePixels()]);
}