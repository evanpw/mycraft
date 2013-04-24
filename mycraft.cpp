#include "block_library.hpp"
#include "chunk.hpp"
#include "coordinate.hpp"
#include "renderer.hpp"
#include "shaders.hpp"
#include "world.hpp"

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
#include <boost/thread.hpp>
#include <glm/gtc/matrix_transform.hpp>

void checkError(const char* where)
{
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		std::cerr << "OpenGL error at " << where << " " << gluErrorString(error) << std::endl;
	}
}

std::ostream& operator<<(std::ostream& out, const glm::vec3& v)
{
	out << "(" << v.x << ", " << v.y << ", " << v.z << ")";
	return out;
}

std::vector<Block> cubes;

Camera camera;
glm::vec3 velocity;

const int INITIAL_WIDTH = 640;
const int INITIAL_HEIGHT = 480;

const float PLAYER_HEIGHT = 1.62;	// Height of eyes in blocks
const float WALKING_SPEED = 4.3;	// Blocks / s
const float FLYING_SPEED = 2.5 * WALKING_SPEED;
const float GRAVITY = 32;			// Blocks / s^2
const float AIR_RESISTANCE = 0.4;	// s^{-1} (of the player)
const float JUMP_VELOCITY = 8.4;	// Blocks / s

// Maximum distance at which one can target (and destroy / place) a block
const float MAX_TARGET_DISTANCE = 10.0f;

World* world;

// Determine the block that the camera is looking directly at
bool castRay(Coordinate& result)
{
	glm::vec3 current = camera.eye;

	glm::mat4 rotation = glm::rotate(glm::mat4(1.0), camera.horizontalAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	rotation = glm::rotate(rotation, camera.verticalAngle, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::vec3 gaze = glm::mat3(rotation) * glm::vec3(0.0f, 0.0f, -1.0f);

	Coordinate currentBlock(current);
	glm::vec3 fractional = glm::fract(current);

	// The direction of travel, in block coordinates
	Coordinate step(glm::sign(gaze));

	// The distance along gaze between consecutive blocks walls
	glm::vec3 delta = glm::length(gaze) / glm::abs(gaze);

	// The distance along gaze to the first block wall
	glm::vec3 distance;
	distance.x = gaze.x < 0 ? fractional.x : 1 - fractional.x;
	distance.y = gaze.y < 0 ? fractional.y : 1 - fractional.y;
	distance.z = gaze.z < 0 ? fractional.z : 1 - fractional.z;
	distance *= delta;

	do
	{
		// Travel the smallest distance necessary to hit the next block

		// Intersects x-wall first
		if (distance.x <= distance.y && distance.x <= distance.z)
		{
			current += distance.x * gaze;
			currentBlock.x += step.x;
			distance -= glm::vec3(distance.x);
			distance.x = delta.x;
		}
		else if (distance.y <= distance.x && distance.y <= distance.z)
		{
			current += distance.y * gaze;
			currentBlock.y += step.y;
			distance -= glm::vec3(distance.y);
			distance.y = delta.y;
		}
		else if (distance.z <= distance.x && distance.z <= distance.y)
		{
			current += distance.z * gaze;
			currentBlock.z += step.z;
			distance -= glm::vec3(distance.z);
			distance.z = delta.z;
		}
		else
		{
			std::cerr << distance << std::endl;

			// Numerical error?
			assert(false);
		}

		// There are never any blocks above a certain y value
		if (glm::length(current - camera.eye) > MAX_TARGET_DISTANCE)
			return false;

	} while (world->isTransparent(currentBlock));

	result = currentBlock;
	return true;
}

Renderer* renderer;

// A movement of 1 pixel corresponds to a rotation of how many degrees?
float rotationSpeed = 0.5;
void GLFWCALL windowResized(int width, int height)
{
	rotationSpeed = 320.0 / width;
	if (renderer)
		renderer->setSize(width, height);
}

bool gravity = true;
bool jump = false;
void GLFWCALL keyCallback(int key, int action)
{
	if (key == 'G' && action == GLFW_PRESS)
	{
		gravity = !gravity;
		std::cout << "Gravity: " << gravity << std::endl;
	}

	if (key == 'I' && action == GLFW_PRESS)
	{
		glm::mat4 rotation = glm::rotate(glm::mat4(1.0), camera.horizontalAngle, glm::vec3(0.0f, 1.0f, 0.0f));
		rotation = glm::rotate(rotation, camera.verticalAngle, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::vec3 gaze = glm::mat3(rotation) * glm::vec3(0.0f, 0.0f, -1.0f);
        std::cout << "Camera location: " << camera.eye.x << ", " << camera.eye.y << ", " << camera.eye.z << std::endl;
        std::cout << "velocity.y = " << velocity.y << std::endl;
        std::cout << "gaze = " << gaze << std::endl;
	}

	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		jump = true;
}

bool mouseCaptured = false;
void GLFWCALL mouseButtonCallback(int button, int action)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		if (mouseCaptured)
		{
			// Determine which block to destroy
			Coordinate targetedBlock;
			bool targeted = castRay(targetedBlock);

			if (targeted)
			{
				const Chunk* chunk = world->removeBlock(targetedBlock);
				renderer->invalidate(chunk);
			}
		}
		else
		{
			mouseCaptured = true;
			glfwDisable(GLFW_MOUSE_CURSOR);
		}
	}
}

size_t frames = 0;
bool running = true;
void fpsThread()
{
	while (running)
	{
		boost::chrono::milliseconds duration(2000);

		size_t lastFrames = frames;
		float lastTime = glfwGetTime();

    	boost::this_thread::sleep_for(duration);

    	// Measure speed
    	size_t currentFrames = frames;
	    float currentTime = glfwGetTime();

        std::cout << float(currentFrames - lastFrames) / (currentTime - lastTime) << " fps" << std::endl;
    }
}

int main()
{
	srand(time(0));

	boost::thread thread(fpsThread);

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

	world = new World;
	renderer = new Renderer(INITIAL_WIDTH, INITIAL_HEIGHT, *world);

	// Make sure that there is a world before we start animating
	world->chunkAt(0, 0);

	// Start up in the air
	camera.eye = glm::vec3(0.0, 64.0, 0.0);

	int lastx, lasty;
	glfwGetMousePos(&lastx, &lasty);

	glfwEnable(GLFW_MOUSE_CURSOR);

	// Loop until the escape key is pressed or the window is closed
	while (glfwGetWindowParam(GLFW_OPENED))
	{
	    // y-coordinate of the player's feet
	    float feetY = camera.eye.y - PLAYER_HEIGHT;

	    // Height of the player's feet above the block directly below
	    float heightAboveBlock = feetY - int(floor(feetY));

	    // Coordinates the block directly below the player's feet
	    Coordinate blockBelow(camera.eye.x, int(feetY) - 1, camera.eye.z);

	    bool inAir = (heightAboveBlock > 1e-3) || !world->isSolid(blockBelow);

		float blocksPerFrame = WALKING_SPEED / 15.0;
		if (!gravity && inAir)
		{
			blocksPerFrame = FLYING_SPEED / 15.0;
		}

		glm::mat4 rotation = glm::rotate(glm::mat4(1.0), camera.horizontalAngle, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec3 facing = glm::mat3(rotation) * glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 right = glm::cross(facing, glm::vec3(0.0f, 1.0f, 0.0f));

		// Rocket mode
		if (glfwGetKey('R') == GLFW_PRESS && gravity && !inAir)
		{
			velocity.y += 10 * JUMP_VELOCITY;
		}

		glm::vec3 step;

		// Jumping
		if (jump)
		{
			jump = false;
			if (!inAir && velocity.y == 0.0f)
				velocity.y = JUMP_VELOCITY;
		}

		// Flying
		if (!gravity && glfwGetKey(GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			velocity.y = 0.0f;
			step.y += blocksPerFrame;
		}

		// Falling
		if (gravity)
		{
			if (inAir)
			{
				velocity -= GRAVITY / 15.0f * glm::vec3(0.0f, 1.0f, 0.0f);
				velocity *= (1 - AIR_RESISTANCE / 15.0);
			}
		}
		else if (glfwGetKey(GLFW_KEY_LSHIFT) == GLFW_PRESS)
		{
			step.y -= blocksPerFrame;
		}

		if (glfwGetKey('W') == GLFW_PRESS)
			step += blocksPerFrame * facing;

		if (glfwGetKey('S') == GLFW_PRESS)
			step -= blocksPerFrame * facing;

		if (glfwGetKey('A') == GLFW_PRESS)
			step -= blocksPerFrame * right;

		if (glfwGetKey('D') == GLFW_PRESS)
			step += blocksPerFrame * right;

		// Actually do the falling / rising
		step += velocity / 15.0f;

		int oldX = camera.eye.x;
		int oldY = camera.eye.y - PLAYER_HEIGHT;
		int oldZ = camera.eye.z;

		// Try moving in the x-direction
		int newX = camera.eye.x + step.x;
		if (world->isSolid(newX, oldY, oldZ))
		{
			step.x = 0;
			velocity.x = 0;
			newX = oldX;
		}

		// Now try the y-direction
		int newY = camera.eye.y + step.y - PLAYER_HEIGHT;
		if (world->isSolid(newX, newY, oldZ))
		{
			step.y = 0;
			camera.eye.y = oldY + PLAYER_HEIGHT + 1e-4;
			velocity.y = 0;
			newY = oldY;
		}

		// Finally, the z-direction
		int newZ = camera.eye.z + step.z;
		if (world->isSolid(newX, newY, newZ))
		{
			step.z = 0;
			velocity.z = 0;
			newZ = oldZ;
		}

		camera.eye += step;

		if (glfwGetKey(GLFW_KEY_ESC) == GLFW_PRESS)
		{
			mouseCaptured = false;
			glfwEnable(GLFW_MOUSE_CURSOR);
		}

		int mouseX, mouseY;
		glfwGetMousePos(&mouseX, &mouseY);

		if (mouseCaptured)
		{
			camera.horizontalAngle -= rotationSpeed * (mouseX - lastx);
			camera.verticalAngle -= rotationSpeed * (mouseY - lasty);

			if (camera.verticalAngle < -90.0) camera.verticalAngle = -90.0;
			if (camera.verticalAngle > 90.0) camera.verticalAngle = 90.0;
		}

		lastx = mouseX;
		lasty = mouseY;

		renderer->render(camera);

		// Display on the screen
		glfwSwapBuffers();
		frames++;
	}

	// Close OpenGL window and terminate glfw
	running = false;
	thread.join();
	glfwTerminate();
	return 0;
}

