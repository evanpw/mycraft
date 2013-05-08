#include "block_library.hpp"
#include "chunk.hpp"
#include "chunk_manager.hpp"
#include "coordinate.hpp"
#include "player.hpp"
#include "ray_caster.hpp"
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
	: m_startTime(-1)
	{}

	void frame()
	{
		float now = glfwGetTime();
		if (m_startTime > 0)
		{
			m_frameTimes.push_back(now - m_startTime);
		}
		m_startTime = now;

		if (m_frameTimes.size() == 100)
		{
			std::sort(m_frameTimes.begin(), m_frameTimes.end());

			float average = std::accumulate(m_frameTimes.begin(), m_frameTimes.end(), 0.0f) / 100;
			float median = m_frameTimes[50];
			float worst = m_frameTimes[99];

			std::cout << "FPS: " << 1 / average << " (average), "
					  << 1 / median << " (median), "
					  << 1 / worst << " (worst)" << std::endl;

			m_frameTimes.clear();
		}

	}

private:
	std::vector<float> m_frameTimes;
	float m_startTime;
};

ChunkManager* chunkManager;
Renderer* renderer;
Player* player;

// A movement of 1 pixel corresponds to a rotation of how many degrees?
float rotationSpeed = 160.0 / INITIAL_WIDTH;
void GLFWCALL windowResizedCallback(int width, int height)
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
		const Camera& camera = player->camera();
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
	if (action == GLFW_PRESS)
	{
		if (mouseCaptured)
		{
			Coordinate targeted, lastOpen;
			bool hit = castRay(player->camera(), *chunkManager, targeted, lastOpen);
			if (hit)
			{
				if (button == GLFW_MOUSE_BUTTON_LEFT)
				{
					chunkManager->removeBlock(targeted);
				}
				else if (button == GLFW_MOUSE_BUTTON_RIGHT)
				{
					// Check that the player will not intersect the new block
					std::vector<Coordinate> locations = player->potentialIntersections();
					if (std::find(locations.begin(), locations.end(), lastOpen) == locations.end())
						chunkManager->createBlock(lastOpen, BlockLibrary::WATER);
				}
			}
		}
		else
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
	glfwSetWindowSizeCallback(windowResizedCallback);
	glfwSetKeyCallback(keyCallback);
	glfwSetMouseButtonCallback(mouseButtonCallback);

	// Ensure we can capture the escape key being pressed below
	glfwEnable(GLFW_STICKY_KEYS);
	glfwEnable(GLFW_STICKY_MOUSE_BUTTONS);

	srand(time(0));
	chunkManager = new ChunkManager(rand());
	renderer = new Renderer(INITIAL_WIDTH, INITIAL_HEIGHT);

	// Start up in the air
	player = new Player(*chunkManager, glm::vec3(0.0, 150.0, 0.0));

	float lastUpdate = glfwGetTime();
	FpsCounter fpsCounter;
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

		std::vector<const Mesh*> visibleMeshes = chunkManager->getVisibleMeshes(player->camera());
		renderer->render(player->camera(), visibleMeshes, player->isUnderwater());

		glfwSwapBuffers();

		fpsCounter.frame();
	}

	// Close OpenGL window and terminate glfw
	glfwTerminate();
	return 0;
}

