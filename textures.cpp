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
#include <png.h>
#include <gl/glfw.h>

uint32_t* readPng(const char* fileName, size_t& width, size_t& height)
{
    png_structp png_ptr;
    png_infop info_ptr;
    FILE *fp;

    if ((fp = fopen(fileName, "rb")) == nullptr)
    {
        std::cerr << "readPng: Unable to open file: " << fileName << std::endl;
        return nullptr;
    }

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (png_ptr == nullptr)
    {
        std::cerr << "readPng: Unable to create read_struct" << std::endl;
        fclose(fp);
        return nullptr;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == nullptr)
    {
        std::cerr << "readPng: Unable to create info struct" << std::endl;
        fclose(fp);
        png_destroy_read_struct(&png_ptr, nullptr, nullptr);
        return nullptr;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        std::cerr << "Error calling setjmp" << std::endl;
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        fclose(fp);
        return nullptr;
    }

    png_init_io(png_ptr, fp);
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_EXPAND, nullptr);

    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);

    png_uint_32 bitdepth   = png_get_bit_depth(png_ptr, info_ptr);
    png_uint_32 channels   = png_get_channels(png_ptr, info_ptr);
    png_uint_32 color_type = png_get_color_type(png_ptr, info_ptr);
    std::cout << fileName << ": " << bitdepth << " " << channels << " " << color_type << std::endl;
    assert(bitdepth == 8);

    uint32_t* data = new uint32_t[width * height];
    png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);
    for (size_t i = 0; i < height; ++i)
    {
        memcpy(&data[i * width], row_pointers[i], width * sizeof(uint32_t));
    }

    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    fclose(fp);

    return data;
}

bool readPng(const char* fileName, uint32_t* buffer, size_t size)
{
    png_structp png_ptr;
    png_infop info_ptr;
    FILE *fp;

    if ((fp = fopen(fileName, "rb")) == nullptr)
    {
        std::cerr << "readPng: Unable to open file: " << fileName << std::endl;
        return false;
    }

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (png_ptr == nullptr)
    {
        std::cerr << "readPng: Unable to create read_struct" << std::endl;
        fclose(fp);
        return false;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == nullptr)
    {
        std::cerr << "readPng: Unable to create info struct" << std::endl;
        fclose(fp);
        png_destroy_read_struct(&png_ptr, nullptr, nullptr);
        return false;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        std::cerr << "Error calling setjmp" << std::endl;
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        fclose(fp);
        return false;
    }

    png_init_io(png_ptr, fp);
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_EXPAND, nullptr);

    size_t width = png_get_image_width(png_ptr, info_ptr);
    size_t height = png_get_image_height(png_ptr, info_ptr);
    if (width != size || height != size)
    {
        std::cerr << "readPng: " << fileName << " is not of the expected size." << std::endl;
        return false;
    }

    png_uint_32 bitdepth   = png_get_bit_depth(png_ptr, info_ptr);
    png_uint_32 channels   = png_get_channels(png_ptr, info_ptr);
    png_uint_32 color_type = png_get_color_type(png_ptr, info_ptr);
    assert(bitdepth == 8 && channels == 4 && color_type == 6);

    png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);
    for (size_t i = 0; i < height; ++i)
    {
        memcpy(&buffer[i * width], row_pointers[i], width * sizeof(uint32_t));
    }

    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    fclose(fp);

    return true;
}

void getPngSize(const char* fileName, size_t& width, size_t& height)
{
    png_structp png_ptr;
    png_infop info_ptr;
    FILE *fp;

    if ((fp = fopen(fileName, "rb")) == nullptr)
    {
        std::cerr << "readPng: Unable to open file: " << fileName << std::endl;
    }

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (png_ptr == nullptr)
    {
        std::cerr << "readPng: Unable to create read_struct" << std::endl;
        fclose(fp);
        return;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == nullptr)
    {
        std::cerr << "readPng: Unable to create info struct" << std::endl;
        fclose(fp);
        png_destroy_read_struct(&png_ptr, nullptr, nullptr);
        return;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        std::cerr << "Error calling setjmp" << std::endl;
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        fclose(fp);
        return;
    }

    png_init_io(png_ptr, fp);
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_EXPAND, nullptr);

    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);

    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    fclose(fp);
}

GLuint createCubeMap(const std::array<const char*, 6>& fileNames)
{
    GLuint texture;
    glGenTextures(1, &texture);

    size_t width, height;
    getPngSize(fileNames[0], width, height);

    // Many block types will have the same textures on some sides of the cube. We
    // use a map here so that we don't read the same pixel data multiple times
    std::map<const char*, std::unique_ptr<uint32_t>> pixelData;
    for (const char* fileName : fileNames)
    {
        if (pixelData.find(fileName) == pixelData.end())
        {
            size_t thisWidth, thisHeight;
            std::unique_ptr<uint32_t> data(readPng(fileName, thisWidth, thisHeight));
            if (!data)
            {
                std::stringstream msg;
                msg << "Problem loading texture " << fileName;
                throw TextureException(msg.str());
            }
            else if (thisWidth != width || thisHeight != height)
            {
                std::stringstream msg;
                msg << "Error: " << fileName << " has size "
                    << "(" << thisWidth << ", " << thisHeight << "), expected "
                    << "(" << width << ", " << height << ")";
                throw TextureException(msg.str());
            }

            pixelData[fileName] = std::move(data);
        }
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    GLenum sides[] =
    {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };

    for (size_t i = 0; i < 6; ++i)
    {
        glTexImage2D(sides[i], 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData[fileNames[i]].get());
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    return texture;
}