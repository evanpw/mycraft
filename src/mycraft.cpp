#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <map>
#include <numeric>
#include <sstream>
#include <vector>

#include "block_library.hpp"
#include "chunk.hpp"
#include "chunk_manager.hpp"
#include "coordinate.hpp"
#include "player.hpp"
#include "ray_caster.hpp"
#include "renderer.hpp"
#include "shaders.hpp"

const int INITIAL_WIDTH = 1920;
const int INITIAL_HEIGHT = 1080;

class FpsCounter {
public:
    FpsCounter() : m_startTime(-1) {}

    void frame() {
        float now = glfwGetTime();
        if (m_startTime > 0) {
            m_frameTimes.push_back(now - m_startTime);
        }
        m_startTime = now;

        if (m_frameTimes.size() == 100) {
            std::sort(m_frameTimes.begin(), m_frameTimes.end());

            float average = std::accumulate(m_frameTimes.begin(), m_frameTimes.end(), 0.0f) / 100;
            float median = m_frameTimes[50];
            float worst = m_frameTimes[99];

            std::cout << "FPS: " << 1 / average << " (average), " << 1 / median << " (median), "
                      << 1 / worst << " (worst)" << std::endl;

            m_frameTimes.clear();
        }
    }

private:
    std::vector<float> m_frameTimes;
    float m_startTime;
};

ChunkManager *chunkManager;
Renderer *renderer;
Player *player;
BlockLibrary::Tag selectedBlock = 0;

// A movement of 1 pixel corresponds to a rotation of how many degrees?
float rotationSpeed = 160.0 / INITIAL_WIDTH;
void windowResizedCallback(GLFWwindow *, int width, int height) {
    rotationSpeed = 160.0 / width;
    if (renderer) renderer->setSize(width, height);
}

void keyCallback(GLFWwindow *, int key, int /*scancode*/, int action, int /*mods*/) {
    // Show some debug info
    if (key == 'I' && action == GLFW_PRESS) {
        const Camera &camera = player->camera();
        std::cout << "Camera location: " << camera.eye.x << ", " << camera.eye.y << ", "
                  << camera.eye.z << std::endl;

        glm::vec3 gaze = camera.gaze();
        std::cout << "Camera gaze = " << gaze.x << ", " << gaze.y << ", " << gaze.z << std::endl;
    } else if ((key == 'B' || key == GLFW_KEY_TAB) && action == GLFW_PRESS) {
        selectedBlock = (selectedBlock + 1) % renderer->blockLibrary().size();
    } else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        player->jump();
    }
}

glm::dvec2 lastMouse;
bool mouseCaptured = false;
void mouseButtonCallback(GLFWwindow *window, int button, int action, int /*mods*/) {
    if (action == GLFW_PRESS) {
        if (mouseCaptured) {
            Coordinate targeted, lastOpen;
            bool hit = castRay(player->camera(), *chunkManager, targeted, lastOpen);
            if (hit) {
                if (button == GLFW_MOUSE_BUTTON_RIGHT ||
                    (button == GLFW_MOUSE_BUTTON_LEFT &&
                     glfwGetKey(window, GLFW_MOD_SUPER) == GLFW_PRESS)) {
                    // Check that the player will not intersect the new block
                    std::vector<Coordinate> locations = player->potentialIntersections();
                    if (std::find(locations.begin(), locations.end(), lastOpen) == locations.end())
                        chunkManager->createBlock(lastOpen, selectedBlock);
                } else if (button == GLFW_MOUSE_BUTTON_LEFT) {
                    chunkManager->removeBlock(targeted);
                }
            }
        } else {
            mouseCaptured = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwGetCursorPos(window, &lastMouse.x, &lastMouse.y);
        }
    }
}

int main() {
    // Initialize glfw
    if (!glfwInit()) {
        std::cerr << "Failed to initialize glfw" << std::endl;
        return 1;
    }

    // Use glfw to open a window
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    GLFWwindow *window;
    if (!(window = glfwCreateWindow(INITIAL_WIDTH, INITIAL_HEIGHT, "MyCraft", nullptr, nullptr))) {
        std::cerr << "Failed to open glfw window." << std::endl;
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);

    // Initialize the OpenGL Extension Wrangler Library
    glewExperimental = true;  // Needed for OpenGL 3.3+
    if (GLenum result = glewInit(); result != GLEW_OK) {
        std::cerr << "Failed to initialize glew: " << glewGetErrorString(result) << std::endl;
        return 1;
    }

    glfwSetWindowSizeCallback(window, windowResizedCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
    glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);

    srand(time(0));
    chunkManager = new ChunkManager(rand());
    renderer = new Renderer(INITIAL_WIDTH, INITIAL_HEIGHT);

    // Start up in the air
    player = new Player(*chunkManager, glm::vec3(0.0, 150.0, 0.0));

    glfwPollEvents();
    glfwGetCursorPos(window, &lastMouse.x, &lastMouse.y);

    float lastUpdate = glfwGetTime();
    FpsCounter fpsCounter;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Determine the time since the last update so we can determine how far
        // the player will travel this frame
        float now = glfwGetTime();
        float elapsed = now - lastUpdate;
        lastUpdate = now;

        if (glfwGetKey(window, 'W') == GLFW_PRESS) player->step(Player::FORWARD);
        if (glfwGetKey(window, 'S') == GLFW_PRESS) player->step(Player::BACKWARD);
        if (glfwGetKey(window, 'A') == GLFW_PRESS) player->step(Player::LEFT);
        if (glfwGetKey(window, 'D') == GLFW_PRESS) player->step(Player::RIGHT);

        /*
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
                player->step(Player::UP);

        if (glfwGetKey(window, GLFW_KEY_LSHIFT) == GLFW_PRESS)
                player->step(Player::DOWN);
        */

        player->update(elapsed);

        if (mouseCaptured && glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            mouseCaptured = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        if (mouseCaptured) {
            glm::dvec2 currentMouse;
            glfwGetCursorPos(window, &currentMouse.x, &currentMouse.y);

            if (currentMouse != lastMouse) {
                player->turnRight(rotationSpeed * (currentMouse.x - lastMouse.x));
                player->tiltUp(rotationSpeed * (currentMouse.y - lastMouse.y));
                lastMouse = currentMouse;
            }
        }

        std::vector<const Mesh *> visibleMeshes = chunkManager->getVisibleMeshes(player->camera());
        renderer->render(player->camera(), visibleMeshes, player->isUnderwater(), selectedBlock);

        glfwSwapBuffers(window);

        fpsCounter.frame();
    }

    // Close OpenGL window and terminate glfw
    glfwTerminate();
    return 0;
}
