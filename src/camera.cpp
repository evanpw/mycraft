#include "camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

glm::vec3 Camera::gaze() const {
    glm::mat4 rotation =
        glm::rotate(glm::mat4(1.0), glm::radians(horizontalAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    rotation = glm::rotate(rotation, glm::radians(verticalAngle), glm::vec3(1.0f, 0.0f, 0.0f));

    return glm::mat3(rotation) * glm::vec3(0.0f, 0.0f, -1.0f);
}
