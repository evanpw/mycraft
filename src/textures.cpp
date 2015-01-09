#define GLM_SWIZZLE
#include <glm/glm.hpp>

#include "exceptions.hpp"
#include "textures.hpp"
#include <array>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <png.h>

PngFile::PngFile(const std::string& fileName)
{
    png_structp png_ptr;
    png_infop info_ptr;
    FILE *fp;

    if ((fp = fopen(fileName.c_str(), "rb")) == nullptr)
    {
        std::stringstream msg;
        msg << "readPng: Unable to open file: " << fileName << std::endl;
        throw std::runtime_error(msg.str());
    }

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr)
    {
        std::stringstream msg;
        msg << "readPng: Unable to create read_struct" << std::endl;
        fclose(fp);
        throw std::runtime_error(msg.str());
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        std::stringstream msg;
        msg << "readPng: Unable to create info struct" << std::endl;
        fclose(fp);
        png_destroy_read_struct(&png_ptr, nullptr, nullptr);
        throw std::runtime_error(msg.str());
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        std::stringstream msg;
        msg << "Error calling setjmp" << std::endl;
        fclose(fp);
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        throw std::runtime_error(msg.str());
    }

    png_init_io(png_ptr, fp);
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_EXPAND, nullptr);

    m_width = png_get_image_width(png_ptr, info_ptr);
    m_height = png_get_image_height(png_ptr, info_ptr);
    m_data.reset(new uint32_t[m_width * m_height]);

    png_uint_32 bitDepth = png_get_bit_depth(png_ptr, info_ptr);
    png_uint_32 channels = png_get_channels(png_ptr, info_ptr);
    assert(bitDepth == 8 && (channels == 4 || channels == 3));

    png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);
    if (channels == 4)
    {
        for (size_t i = 0; i < m_height; ++i)
        {
            memcpy(&m_data[i * m_width], row_pointers[i], m_width * sizeof(uint32_t));
        }
    }
    else if (channels == 3)
    {
        for (size_t i = 0; i < m_height; ++i)
        {
            for (size_t j = 0; j < m_width; ++j)
            {
                uint32_t color = *reinterpret_cast<uint32_t*>(&row_pointers[i][3 * j]);

                // The upper byte actually comes from a different pixel. Just set the
                // alpha to full
                color |= (0xFF << 24);
                m_data[i * m_width + j] = color;
            }
        }
    }

    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    fclose(fp);
}

glm::vec4 split(uint32_t color)
{
    uint8_t a = (color & 0xFF000000) >> 24;
    uint8_t b = (color & 0xFF0000) >> 16;
    uint8_t g = (color & 0xFF00) >> 8;
    uint8_t r = (color & 0xFF);

    return glm::vec4(r, g, b, a) / 255.0;
}

uint32_t unsplit(const glm::vec4& color)
{
    glm::ivec4 scaled(255.0 * color);
    return (scaled.a << 24) + (scaled.b << 16) + (scaled.g << 8) + scaled.r;
}

glm::vec4 blend(glm::vec4 source, glm::vec4 dest)
{
    glm::vec4 result;

    result.a = source.a + dest.a * (1 - source.a);
    if (result.a == 0.0f)
        return glm::vec4(0.0f);

    result.rgb = (source.a * source.rgb + dest.a * dest.rgb * (1 - source.a)) / result.a;
    return result;
}

void PngFile::tint(const glm::vec3& color)
{
    // Colorize the top texture
    for (size_t i = 0; i < m_width * m_height; ++i)
    {
        glm::vec4 pixel = split(m_data[i]);
        pixel *= glm::vec4(color, 1.0);
        m_data[i] = unsplit(pixel);
    }
}

void PngFile::overlayWith(const PngFile& other)
{
    assert(width() == other.width() && height() == other.height());

    uint32_t* otherBuffer = other.buffer();

    for (size_t i = 0; i < m_width * m_height; ++i)
    {
        glm::vec4 myColor = split(m_data[i]);
        glm::vec4 otherColor = split(otherBuffer[i]);
        m_data[i] = unsplit(blend(myColor, otherColor));
    }

}

void PngFile::copyTo(uint32_t* buffer)
{
    memcpy(buffer, m_data.get(), sizeof(uint32_t) * m_width * m_height);
}