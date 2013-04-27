#include "block_library.hpp"
#include "chunk.hpp"
#include "chunk_manager.hpp"
#include "coordinate.hpp"
#include "player.hpp"
#include "renderer.hpp"
#include "shaders.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>

#include <GL/glew.h>
#include <GL/glfw.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

const int INITIAL_WIDTH = 640;
const int INITIAL_HEIGHT = 480;

class FpsCounter
{
public:
	FpsCounter()
	: m_startTime(glfwGetTime()), m_frames(0)
	{}

	void frame()
	{
		++m_frames;
	}

	float elapsedTime()
	{
		return glfwGetTime() - m_startTime;
	}

	float reset()
	{
		float now = glfwGetTime();
		float fps = m_frames / (now - m_startTime);

		m_startTime = now;
		m_frames = 0;

		return fps;
	}

private:
	float m_startTime;
	unsigned int m_frames;
};

ChunkManager* chunkManager;
Renderer* renderer;
Camera camera;
Player* player;

// A movement of 1 pixel corresponds to a rotation of how many degrees?
float rotationSpeed = 160.0 / INITIAL_WIDTH;
void GLFWCALL windowResized(int width, int height)
{
	rotationSpeed = 160.0 / width;
	if (renderer)
		renderer->setSize(width, height);
}

void GLFWCALL keyCallback(int key, int action)
{
	// Show some debug info
	if (key == 'I' && action == GLFW_PRESS)
	{
        std::cout << "Camera location: " << camera.eye.x << ", " << camera.eye.y << ", " << camera.eye.z << std::endl;

        glm::vec3 gaze = camera.gaze();
        std::cout << "Camera gaze = " << gaze.x << ", " << gaze.y << ", " << gaze.z << std::endl;
	}
	else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		player->jump();
	}
}

bool mouseCaptured = false;
void GLFWCALL mouseButtonCallback(int button, int action)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		if (!mouseCaptured)
		{
			mouseCaptured = true;
			glfwDisable(GLFW_MOUSE_CURSOR);
			glfwSetMousePos(renderer->width() / 2, renderer->height() / 2);
		}
	}
}

int main()
{
	// Initialize glfw
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize glfw" << std::endl;
		return 1;
	}

	// Use glfw to open a window
	glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4); // 4 samples per pixel
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	if(!glfwOpenWindow(INITIAL_WIDTH, INITIAL_HEIGHT, 0, 0, 0, 0, 32, 0, GLFW_WINDOW))
	{
		std::cerr << "Failed to open glfw window." << std::endl;
		glfwTerminate();
		return 1;
	}

	// Initialize the OpenGL Extension Wrangler Library
	glewExperimental = true; // Needed for OpenGL 3.3+
	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Failed to initialize glew" << std::endl;
		return 1;
	}

	glfwSetWindowTitle("MyCraft");
	glfwSetWindowSizeCallback(windowResized);
	glfwSetKeyCallback(keyCallback);
	glfwSetMouseButtonCallback(mouseButtonCallback);

	// Ensure we can capture the escape key being pressed below
	glfwEnable(GLFW_STICKY_KEYS);
	glfwEnable(GLFW_STICKY_MOUSE_BUTTONS);

	srand(time(0));
	chunkManager = new ChunkManager(rand());
	renderer = new Renderer(INITIAL_WIDTH, INITIAL_HEIGHT);

	// Start up in the air
	player = new Player(*chunkManager, glm::vec3(0.0, 32.0, 0.0));

	float lastUpdate = glfwGetTime();

	FpsCounter fpsCounter;
	fpsCounter.reset();

	while (glfwGetWindowParam(GLFW_OPENED))
	{
		// Determine the time since the last update so we can determine how far
		// the player will travel this frame
		float now = glfwGetTime();
		float elapsed = now - lastUpdate;
		lastUpdate = now;

		glm::vec3 step;
		if (glfwGetKey('W') == GLFW_PRESS)
			player->step(Player::FORWARD);

		if (glfwGetKey('S') == GLFW_PRESS)
			player->step(Player::BACKWARD);

		if (glfwGetKey('A') == GLFW_PRESS)
			player->step(Player::LEFT);

		if (glfwGetKey('D') == GLFW_PRESS)
			player->step(Player::RIGHT);

		/*
		if (glfwGetKey(GLFW_KEY_SPACE) == GLFW_PRESS)
			player->step(Player::UP);

		if (glfwGetKey(GLFW_KEY_LSHIFT) == GLFW_PRESS)
			player->step(Player::DOWN);
		*/

		player->update(elapsed);

		if (glfwGetKey(GLFW_KEY_ESC) == GLFW_PRESS)
		{
			mouseCaptured = false;
			glfwEnable(GLFW_MOUSE_CURSOR);
		}

		if (mouseCaptured)
		{
			glm::ivec2 currentMouse;
			glfwGetMousePos(&currentMouse.x, &currentMouse.y);

			player->turnRight(rotationSpeed * (currentMouse.x - (renderer->width() / 2)));
			player->tiltUp(rotationSpeed * (currentMouse.y - (renderer->height() / 2)));
			glfwSetMousePos(renderer->width() / 2, renderer->height() / 2);
		}

		std::vector<Mesh*> visibleMeshes = chunkManager->getVisibleMeshes(player->camera());
		renderer->renderMeshes(player->camera(), visibleMeshes);
		glfwSwapBuffers();

		fpsCounter.frame();
		if (fpsCounter.elapsedTime() > 1.0f)
		{
			std::cout << "FPS: " << fpsCounter.reset() << std::endl;
		}
	}

	// Close OpenGL window and terminate glfw
	glfwTerminate();
	return 0;
}

