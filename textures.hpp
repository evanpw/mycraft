#ifndef TEXTURES_HPP
#define TEXTURES_HPP

#include <cstdint>
#include <memory>
#include <GL/glew.h>
#include <glm/glm.hpp>

bool readPng(const std::string fileName, uint32_t* buffer, size_t size);

class PngFile
{
public:
	PngFile(const std::string& fileName);

	size_t width() const { return m_width; }
	size_t height() const { return m_height; }
	uint32_t* buffer() const { return m_data.get(); }

	void tint(const glm::vec3& color);

	// Place another texture on the top of this one, with
	// correct alpha blending
	void overlayWith(const PngFile& other);

	void copyTo(uint32_t* buffer);

private:
	size_t m_width, m_height;
	std::unique_ptr<uint32_t[]> m_data;
};

#endif