#include "mesh.hpp"

#include <glm/gtc/type_ptr.hpp>

void copyVector(GLfloat* dest, const glm::vec3& source) {
    memcpy(dest, glm::value_ptr(source), 3 * sizeof(GLfloat));
}