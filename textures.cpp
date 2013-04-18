#include "textures.hpp"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <png.h>
#include <gl/glfw.h>

uint32_t* readPng(const char* fileName, int* outWidth, int* outHeight)
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

    png_uint_32 width = png_get_image_width(png_ptr, info_ptr);
    png_uint_32 height = png_get_image_height(png_ptr, info_ptr);

    png_uint_32 bitdepth   = png_get_bit_depth(png_ptr, info_ptr);
    png_uint_32 channels   = png_get_channels(png_ptr, info_ptr);
    png_uint_32 color_type = png_get_color_type(png_ptr, info_ptr);
    std::cout << fileName << ": " << bitdepth << " " << channels << " " << color_type << std::endl;
    assert(bitdepth == 8);

    *outWidth = width;
    *outHeight = height;

    png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);

    uint32_t* data = new uint32_t[width * height];
    for (size_t i = 0; i < height; ++i)
    {
    	// Invert the y-coordinate so that (0, 0) is the bottom-left corner
        memcpy(&data[i * width], row_pointers[height - 1 - i], width * sizeof(uint32_t));
    }

    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    fclose(fp);

    return data;
}

GLuint makeTexture(const char* filename)
{
    int width, height;
    uint32_t* pixels = readPng(filename, &width, &height);
    if (pixels == 0)
    {
    	std::cerr << "Problem loading texture " << filename << std::endl;
    	return 0;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(
        GL_TEXTURE_2D,				// Target
        0,           				// Level of detail
        GL_RGBA8,                   // Internal format
        width, height, 0,           // Width, Height, and Border
        GL_RGBA, GL_UNSIGNED_BYTE,  // External format, type
        pixels                      // Pixel data
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);

    delete[] pixels;
    return texture;
}