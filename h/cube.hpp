#ifndef CUBE_HPP
#define CUBE_HPP

#include <array>
#include <glm/glm.hpp>

struct CubeVertex {
    glm::vec3 position, normal;
    glm::vec2 texCoord;
};

extern const std::array<CubeVertex, 36> cubeMesh;

#endif