#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <GL/glew.h>

#include <map>
#include <memory>

#include "block_library.hpp"
#include "camera.hpp"
#include "chunk.hpp"
#include "mesh.hpp"

class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();

    Renderer(const Renderer& other) = delete;
    Renderer& operator=(const Renderer& other) = delete;

    void render(const Camera& camera, const std::vector<const Mesh*>& meshes, bool underwater,
                BlockLibrary::Tag selected);

    void setSize(int width, int height);
    int width() const { return m_width; }
    int height() const { return m_height; }

    const BlockLibrary& blockLibrary() const { return *m_blockLibrary; }

private:
    void buildViewProjectionMatrix(const Camera& camera) const;

    int m_width, m_height;
    glm::mat4 m_projection;

    std::unique_ptr<BlockLibrary> m_blockLibrary;

    GLuint m_vertexArray;

    // Shader for rendering chunks of terrain
    struct {
        GLuint programId;

        // Shader input variables
        GLint position, texCoord, lighting;

        // Shader uniform variables
        GLint modelMatrix, vpMatrix, highlight, textureSampler;
        GLint resolution, sunPosition, brightness;
    } m_chunkShader;

    // Shader program for tinting the screen (for example,
    // when underwater).
    void tintScreen(const glm::vec3& color);
    struct {
        GLuint programId;

        // Input variables
        GLint position;

        // Uniform variables
        GLint color;

        // Stores the vertices of a quad which covers the entire screen.
        GLuint vbo;
    } m_tintShader;

    // Shader program for displaying the current selected block
    void drawBlock(BlockLibrary::Tag blockType);
    struct {
        GLuint programId;

        // Input variables
        GLint position, texCoord;

        // Uniform variables
        GLint textureSampler, projection, blockType;

        // Stores the block vertices
        GLuint vbo;
    } m_blockShader;
};

#endif
