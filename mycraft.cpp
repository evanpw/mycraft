#include "block_library.hpp"
#include "chunk.hpp"
#include "coordinate.hpp"
#include "noise.hpp"
#include "shaders.hpp"

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
#include <glm/gtc/matrix_transform.hpp>

std::ostream& operator<<(std::ostream& out, const glm::vec3& v)
{
	out << "(" << v.x << ", " << v.y << ", " << v.z << ")";
	return out;
}

struct VertexData
{
	GLfloat position[3];
	GLfloat texCoord[3];
	GLfloat normal[3];
};

const VertexData cubeData[] =
{
	// Front face
	{{0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, SIDE}, {0.0f, 0.0f, 1.0f}},
	{{1.0f, 0.0f, 1.0f}, {1.0f, 0.0f, SIDE}, {0.0f, 0.0f, 1.0f}},
	{{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, SIDE}, {0.0f, 0.0f, 1.0f}},

	{{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, SIDE}, {0.0f, 0.0f, 1.0f}},
	{{1.0f, 0.0f, 1.0f}, {1.0f, 0.0f, SIDE}, {0.0f, 0.0f, 1.0f}},
	{{0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, SIDE}, {0.0f, 0.0f, 1.0f}},



	// Right face
	{{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, SIDE}, {1.0f, 0.0f, 0.0f}},
	{{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, SIDE}, {1.0f, 0.0f, 0.0f}},
	{{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, SIDE}, {1.0f, 0.0f, 0.0f}},

	{{1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, SIDE}, {1.0f, 0.0f, 0.0f}},
	{{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, SIDE}, {1.0f, 0.0f, 0.0f}},
	{{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, SIDE}, {1.0f, 0.0f, 0.0f}},



	// Back face
	{{0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, SIDE}, {0.0f, 0.0f, -1.0f}},
	{{1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, SIDE}, {0.0f, 0.0f, -1.0f}},
	{{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, SIDE}, {0.0f, 0.0f, -1.0f}},

	{{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, SIDE}, {0.0f, 0.0f, -1.0f}},
	{{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, SIDE}, {0.0f, 0.0f, -1.0f}},
	{{1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, SIDE}, {0.0f, 0.0f, -1.0f}},



	// Left face
	{{0.0f, 1.0f, 1.0f}, {1.0f, 1.0f, SIDE}, {-1.0f, 0.0f, 0.0f}},
	{{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, SIDE}, {-1.0f, 0.0f, 0.0f}},
	{{0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, SIDE}, {-1.0f, 0.0f, 0.0f}},

	{{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, SIDE}, {-1.0f, 0.0f, 0.0f}},
	{{0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, SIDE}, {-1.0f, 0.0f, 0.0f}},
	{{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, SIDE}, {-1.0f, 0.0f, 0.0f}},



	// Top face
	{{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, TOP}, {0.0f, 1.0f, 0.0f}},
	{{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, TOP}, {0.0f, 1.0f, 0.0f}},
	{{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, TOP}, {0.0f, 1.0f, 0.0f}},

	{{0.0f, 1.0f, 1.0f}, {0.0f, 0.0f, TOP}, {0.0f, 1.0f, 0.0f}},
	{{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, TOP}, {0.0f, 1.0f, 0.0f}},
	{{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, TOP}, {0.0f, 1.0f, 0.0f}},



	// Bottom face
	{{1.0f, 0.0f, 1.0f}, {1.0f, 1.0f, BOTTOM}, {0.0f, -1.0f, 0.0f}},
	{{0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, BOTTOM}, {0.0f, -1.0f, 0.0f}},
	{{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, BOTTOM}, {0.0f, -1.0f, 0.0f}},

	{{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, BOTTOM}, {0.0f, -1.0f, 0.0f}},
	{{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, BOTTOM}, {0.0f, -1.0f, 0.0f}},
	{{0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, BOTTOM}, {0.0f, -1.0f, 0.0f}}
};

std::vector<Block> cubes;

struct Camera
{
	Camera() : horizontalAngle(0), verticalAngle(0) {}

	glm::vec3 eye;	// Camera location in world coordinates
	float horizontalAngle, verticalAngle;
};

Camera camera;
glm::vec3 velocity;

Chunk* chunk;

int windowWidth = 640;
int windowHeight = 480;

const float PLAYER_HEIGHT = 1.62;	// Height of eyes in blocks
const float WALKING_SPEED = 4.3;	// Blocks / s
const float FLYING_SPEED = 2.5 * WALKING_SPEED;
const float GRAVITY = 32;			// Blocks / s^2
const float AIR_RESISTANCE = 0.4;	// s^{-1} (of the player)
const float JUMP_VELOCITY = 8.4;	// Blocks / s

// Maximum distance at which one can target (and destroy / place) a block
const float MAX_TARGET_DISTANCE = 10.0f;

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

	} while (chunk->isTransparent(currentBlock));

	result = currentBlock;
	return true;
}

class Renderer
{
public:
	Renderer(int width, int height);

	Renderer(const Renderer& other) = delete;
	Renderer& operator=(const Renderer& other) = delete;

	void render(const Chunk& chunk, const Camera& camera) const;
	void setSize(int width, int height);

private:
	void buildViewProjectionMatrix(const Camera& camera) const;

	int m_width, m_height;

	std::unique_ptr<BlockLibrary> m_blockLibrary;

	GLuint m_vertexBuffer;
	GLuint m_programId;

	// Shader input variables
	GLint m_position, m_texCoord, m_normal;

	// Shader uniform variables
	GLint m_modelMatrix, m_vpMatrix, m_highlight, m_textureSampler, m_resolution;
};

Renderer::Renderer(int width, int height)
: m_blockLibrary(new BlockLibrary)
{
	setSize(width, height);

	// Sky color
	glClearColor(0.8f, 0.8f, 1.0f, 0.0f);

	// Cull back faces
	glEnable(GL_CULL_FACE);

	// We don't sort blocks ourselves, so we need depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// For transparent blocks
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Create a vertex array object
	GLuint vertexArray;
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);

	// Create a vertex buffer and send the vertex data
	glGenBuffers(1, &m_vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeData), cubeData, GL_STATIC_DRAW);

	// Load, compile, and link the shaders
	GLuint vertexShader = loadShader("vertex.glsl", GL_VERTEX_SHADER);
	GLuint fragmentShader = loadShader("fragment.glsl", GL_FRAGMENT_SHADER);
	m_programId = linkShaders(vertexShader, fragmentShader);

	// Get the attribute id of the input variables
	m_position = glGetAttribLocation(m_programId, "position");
	m_texCoord = glGetAttribLocation(m_programId, "texCoord");
	m_normal = glGetAttribLocation(m_programId, "normal");

	// Get ids for the uniform variables
	m_vpMatrix = glGetUniformLocation(m_programId, "vpMatrix");
	m_modelMatrix = glGetUniformLocation(m_programId, "modelMatrix");
	m_textureSampler = glGetUniformLocation(m_programId, "textureSampler");
	m_highlight = glGetUniformLocation(m_programId, "highlight");
	m_resolution = glGetUniformLocation(m_programId, "resolution");
}

void Renderer::render(const Chunk& chunk, const Camera& camera) const
{
	// Determine which block (if any) to highlight
	Coordinate targetedBlock;
	bool targeted = castRay(targetedBlock);

	glUseProgram(m_programId);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);

	glVertexAttribPointer(m_position, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, position));
	glEnableVertexAttribArray(m_position);

	glVertexAttribPointer(m_texCoord, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, texCoord));
	glEnableVertexAttribArray(m_texCoord);

	glVertexAttribPointer(m_normal, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, normal));
	glEnableVertexAttribArray(m_normal);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(m_textureSampler, 0);
	glUniform2f(m_resolution, windowWidth, windowHeight);

	// All of the blocks have the same view and projection matrices
	buildViewProjectionMatrix(camera);

	// Fill the screen with sky color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (auto& cubeSet : chunk.liveBlocks())
	{
		// A cube set is the collection of all live blocks with the same block type
		BlockLibrary::Tag blockType = cubeSet.first;
	    glBindTexture(GL_TEXTURE_2D_ARRAY, m_blockLibrary->get(blockType).texture);

		for (auto& cube : cubeSet.second)
		{
			// All of the blocks use the same cube mesh, in model coordinates. We pass the location
			// and the shader does the translation.
			glm::mat4 model = glm::translate(glm::mat4(1.0f), cube->location.vec3());
			glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, &model[0][0]);

			// We highlight the block that the player is looking directly at
			if (targeted && cube->location == targetedBlock)
				glUniform1i(m_highlight, GL_TRUE);
			else
				glUniform1i(m_highlight, GL_FALSE);

			// Draw the block
			glDrawArrays(GL_TRIANGLES, 0, 3 * 12);
		}
	}

	// Good OpenGL hygiene
	glDisableVertexAttribArray(m_position);
	glDisableVertexAttribArray(m_texCoord);
	glDisableVertexAttribArray(m_normal);
}

void Renderer::setSize(int width, int height)
{
	m_width = width;
	m_height = height;
	glViewport(0, 0, m_width, m_height);
}

void Renderer::buildViewProjectionMatrix(const Camera& camera) const
{
	float aspectRatio = float(m_width) / m_height;

	glm::mat4 projection = glm::perspective(
		45.0f,			// Field of view
		aspectRatio,
		0.1f,			// Near clipping plane
		256.0f			// Far clipping plane
	);

	glm::mat4 rotation = glm::rotate(glm::mat4(1.0), camera.horizontalAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	rotation = glm::rotate(rotation, camera.verticalAngle, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::vec3 gaze = glm::mat3(rotation) * glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 up = glm::mat3(rotation) * glm::vec3(0.0, 1.0f, 0.0f);

	glm::mat4 view = glm::lookAt(camera.eye, camera.eye + gaze, up);

	glm::mat4 mvp = projection * view;
	glUniformMatrix4fv(m_vpMatrix, 1, GL_FALSE, &mvp[0][0]);
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
				chunk->removeBlock(targetedBlock);
			}
		}
		else
		{
			mouseCaptured = true;
			glfwDisable(GLFW_MOUSE_CURSOR);
		}
	}
}

int main()
{
	srand(time(0));

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
	if(!glfwOpenWindow(640, 480, 0, 0, 0, 0, 32, 0, GLFW_WINDOW))
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

	renderer = new Renderer(windowWidth, windowHeight);
	chunk = new Chunk;

	// Start up in the air
	float location = (Chunk::SIZE / 2.0) + 0.5;
	camera.eye = glm::vec3(location, 64.0, location);

	int lastx, lasty;
	glfwGetMousePos(&lastx, &lasty);

	glfwEnable(GLFW_MOUSE_CURSOR);

	float lastTime = glfwGetTime();
	float lastFrame = 0.0f;
 	int nbFrames = 0;

	// Loop until the escape key is pressed or the window is closed
	while (glfwGetWindowParam(GLFW_OPENED))
	{
		// Measure speed
	    float currentTime = glfwGetTime();
	    if (currentTime - lastTime >= 1.0)
	    {
	        std::cout << float(nbFrames) / (currentTime - lastTime) << " fps" << std::endl;
	        std::cout << "Camera location: " << camera.eye.x << ", " << camera.eye.y << ", " << camera.eye.z << std::endl;
	        std::cout << "velocity.y = " << velocity.y << std::endl;
	        nbFrames = 0;
	        lastTime = currentTime;
	    }

	    // y-coordinate of the player's feet
	    float feetY = camera.eye.y - PLAYER_HEIGHT;

	    // Height of the player's feet above the block directly below
	    float heightAboveBlock = feetY - static_cast<int>(feetY);

	    // Coordinates the block directly below the player's feet
	    Coordinate blockBelow(camera.eye.x, int(feetY) - 1, camera.eye.z);

	    bool inAir = (heightAboveBlock > 1e-3) || !chunk->isSolid(blockBelow);

		float blocksPerFrame = WALKING_SPEED * lastFrame;
		if (!gravity && inAir)
		{
			blocksPerFrame = FLYING_SPEED * lastFrame;
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
				velocity -= GRAVITY * lastFrame * glm::vec3(0.0f, 1.0f, 0.0f);
				velocity *= (1 - AIR_RESISTANCE * lastFrame);
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
		step += velocity * lastFrame;

		int oldX = camera.eye.x;
		int oldY = camera.eye.y - PLAYER_HEIGHT;
		int oldZ = camera.eye.z;

		// Try moving in the x-direction
		int newX = camera.eye.x + step.x;
		if (chunk->isSolid(newX, oldY, oldZ))
		{
			step.x = 0;
			velocity.x = 0;
			newX = oldX;
		}

		// Now try the y-direction
		int newY = camera.eye.y + step.y - PLAYER_HEIGHT;
		if (chunk->isSolid(newX, newY, oldZ))
		{
			step.y = 0;
			camera.eye.y = oldY + PLAYER_HEIGHT + 1e-4;
			velocity.y = 0;
			newY = oldY;
		}

		// Finally, the z-direction
		int newZ = camera.eye.z + step.z;
		if (chunk->isSolid(newX, newY, newZ))
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

		renderer->render(*chunk, camera);

		Coordinate targetedBlock;
		castRay(targetedBlock);

		// Display on the screen
		glfwSwapBuffers();
		nbFrames++;
		lastFrame = glfwGetTime() - currentTime;
	}

	// Close OpenGL window and terminate glfw
	glfwTerminate();
	return 0;
}

