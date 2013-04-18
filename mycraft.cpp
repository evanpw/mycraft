#include "block_library.hpp"
#include "chunk.hpp"
#include "coordinate.hpp"
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

BlockLibrary* blockLibrary;

struct VertexData
{
	GLfloat position[3];
	GLfloat uv[2];
	GLfloat normal[3];
};

// First three elements of each sub-array are vertex position in model coordinates,
// middle two are the texture coordinates, and last three are the normals
const VertexData cubeData[] =
{
	// Front face
	{{0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
	{{1.0f, 0.0f, 1.0f}, {0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	{{1.0f, 1.0f, 1.0f}, {0.5f, 1.0f}, {0.0f, 0.0f, 1.0f}},

	{{0.0f, 0.0f, 1.0f}, {0.0f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	{{1.0f, 0.0f, 1.0f}, {0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	{{0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},



	// Right face
	{{1.0f, 1.0f, 0.0f}, {0.5f, 1.0f}, {1.0f, 0.0f, 0.0f}},
	{{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
	{{1.0f, 0.0f, 0.0f}, {0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},

	{{1.0f, 0.0f, 1.0f}, {0.0f, 0.5f}, {1.0f, 0.0f, 0.0f}},
	{{1.0f, 0.0f, 0.0f}, {0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
	{{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},



	// Back face
	{{0.0f, 1.0f, 0.0f}, {0.5f, 1.0f}, {0.0f, 0.0f, -1.0f}},
	{{1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},
	{{0.0f, 0.0f, 0.0f}, {0.5f, 0.5f}, {0.0f, 0.0f, -1.0f}},

	{{1.0f, 0.0f, 0.0f}, {0.0f, 0.5f}, {0.0f, 0.0f, -1.0f}},
	{{0.0f, 0.0f, 0.0f}, {0.5f, 0.5f}, {0.0f, 0.0f, -1.0f}},
	{{1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},



	// Left face
	{{0.0f, 1.0f, 1.0f}, {0.5f, 1.0f}, {-1.0f, 0.0f, 0.0f}},
	{{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}},
	{{0.0f, 0.0f, 1.0f}, {0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}},

	{{0.0f, 0.0f, 0.0f}, {0.0f, 0.5f}, {-1.0f, 0.0f, 0.0f}},
	{{0.0f, 0.0f, 1.0f}, {0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}},
	{{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}},



	// Top face
	{{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
	{{0.0f, 1.0f, 0.0f}, {0.5f, 1.0f}, {0.0f, 1.0f, 0.0f}},
	{{1.0f, 1.0f, 1.0f}, {1.0f, 0.5f}, {0.0f, 1.0f, 0.0f}},

	{{0.0f, 1.0f, 1.0f}, {0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
	{{1.0f, 1.0f, 1.0f}, {1.0f, 0.5f}, {0.0f, 1.0f, 0.0f}},
	{{0.0f, 1.0f, 0.0f}, {0.5f, 1.0f}, {0.0f, 1.0f, 0.0f}},



	// Bottom face
	{{1.0f, 0.0f, 1.0f}, {0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}},
	{{0.0f, 0.0f, 1.0f}, {0.0f, 0.5f}, {0.0f, -1.0f, 0.0f}},
	{{1.0f, 0.0f, 0.0f}, {0.5f, 0.0f}, {0.0f, -1.0f, 0.0f}},

	{{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},
	{{1.0f, 0.0f, 0.0f}, {0.5f, 0.0f}, {0.0f, -1.0f, 0.0f}},
	{{0.0f, 0.0f, 1.0f}, {0.0f, 0.5f}, {0.0f, -1.0f, 0.0f}}
};


GLuint vertexBuffer, elementBuffer;
GLint position, modelMatrix, vpMatrix, vertexUv, normal, textureSampler, highlight, resolution;
GLuint programId;

std::vector<Block> cubes;

struct Camera
{
	Camera() : horizontalAngle(0), verticalAngle(0) {}

	glm::vec3 eye;	// Camera location in world coordinates
	float horizontalAngle, verticalAngle;
};

Camera camera;
glm::vec3 velocity;

// Generate noise via the midpoint displacement algorithm
// http://www.lighthouse3d.com/opengl/terrain/index.php3?mpd2
class Noise
{
public:
	Noise(uint8_t k, int range = 16)
	: m_size((1 << k) + 1)
	{
		m_grid.resize(m_size);
		for (size_t i = 0; i < m_size; ++i)
		{
			m_grid[i].resize(m_size);
		}

		// Fill in the corners first
		unsigned int scale = 1 << k;
		unsigned int d = range / 4;
		m_grid[0][0] = fuzz(d);
		m_grid[0][scale] = fuzz(d);
		m_grid[scale][0] = fuzz(d);
		m_grid[scale][scale] = fuzz(d);

		while (scale > 1)
		{
			scale /= 2;

			// Center of squares
			for (size_t i = scale; i < m_size; i += scale * 2)
				for (size_t j = scale; j < m_size; j += scale * 2)
					m_grid[i][j] = averageCorners(i, j, scale) + fuzz(d);

			// Sides of squares
			for (size_t i = 0; i < m_size; i += scale * 2)
				for (size_t j = scale; j < m_size; j += scale * 2)
					m_grid[i][j] = averageNeighbors(i, j, scale) + fuzz(d);

			for (size_t i = scale; i < m_size; i += scale * 2)
				for (size_t j = 0; j < m_size; j += scale * 2)
					m_grid[i][j] = averageNeighbors(i, j, scale) + fuzz(d);

			d /= 2;
		}

		// Shift upward to yield positive numbers
		for (size_t i = 0; i < m_size; ++i)
		{
			for (size_t j = 0; j < m_size; ++j)
			{
				m_grid[i][j] += (range / 2);
				assert(m_grid[i][j] >= 0 && m_grid[i][j] < range);
			}
		}
	}

	int operator()(size_t i, size_t j)
	{
		return m_grid[i][j];
	}

	const std::vector<std::vector<int>> grid() const
	{
		return m_grid;
	}

	int fuzz(unsigned int range)
	{
		return (rand() % (2 * range + 1)) - range;
	}

	int averageCorners(size_t i, size_t j, unsigned int scale)
	{
		return (m_grid[i - scale][j - scale] +
				m_grid[i + scale][j - scale] +
				m_grid[i - scale][j + scale] +
				m_grid[i + scale][j - scale]) / 4;
	}

	int averageNeighbors(size_t i, size_t j, unsigned int scale)
	{
		int total = 0;
		int count = 0;

		if (i >= scale)
		{
			total += m_grid[i - scale][j];
			++count;
		}

		if (i + scale < m_size)
		{
			total += m_grid[i + scale][j];
			++count;
		}

		if (j >= scale)
		{
			total += m_grid[i][j - scale];
			++count;
		}

		if (j + scale < m_size)
		{
			total += m_grid[i][j + scale];
			++count;
		}

		return total / count;
	}

private:
	size_t m_size;
	std::vector<std::vector<int>> m_grid;
};

Noise terrainHeight(Chunk::BITS, Chunk::SIZE / 4);
Chunk* chunk;

void initialize()
{
	// Create a vertex array object
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create a vertex buffer and send the vertex data
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeData), cubeData, GL_STATIC_DRAW);

	// Load, compile, and link the shaders
	GLuint vertexShader = loadShader("vertex.glsl", GL_VERTEX_SHADER);
	GLuint fragmentShader = loadShader("fragment.glsl", GL_FRAGMENT_SHADER);
	programId = linkShaders(vertexShader, fragmentShader);

	// Get the attribute id of the input variable "position" to the vertex shader
	position = glGetAttribLocation(programId, "position");
	vertexUv = glGetAttribLocation(programId, "vertexUv");
	normal = glGetAttribLocation(programId, "normal");

	// Get ids for the uniform variables
	vpMatrix = glGetUniformLocation(programId, "vpMatrix");
	modelMatrix = glGetUniformLocation(programId, "modelMatrix");
	textureSampler = glGetUniformLocation(programId, "textureSampler");
	highlight = glGetUniformLocation(programId, "highlight");
	resolution = glGetUniformLocation(programId, "resolution");

	chunk = new Chunk(terrainHeight.grid());

	// Start up in the air
	float location = (Chunk::SIZE / 2.0) + 0.5;
	camera.eye = glm::vec3(location, 64.0, location);

	blockLibrary = new BlockLibrary();
}

int windowWidth = 640;
int windowHeight = 480;

void buildViewProjectionMatrix()
{
	float aspectRatio = static_cast<float>(windowWidth) / windowHeight;

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
	glUniformMatrix4fv(
		vpMatrix,	// Id of this uniform variable
		1,			// Number of matrices
		GL_FALSE,	// Transpose
		&mvp[0][0]	// The location of the data
	);
}

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
			std::cout << distance << std::endl;

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

void render(const Chunk& chunk)
{
	// Determine which block (if any) to highlight
	Coordinate targetedBlock;
	bool targeted = castRay(targetedBlock);

	glUseProgram(programId);

	// Bind vertexBuffer to GL_ARRAY_BUFFER
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	glVertexAttribPointer(
		position,           	// This is the id we got for the "position" input variable
	   	3,                  	// number of components
	   	GL_FLOAT,           	// type
	   	GL_FALSE,           	// normalize?
	   	sizeof(VertexData),    // skip the two uv coordinates to get to the next position
	   	(void*)offsetof(VertexData, position)
	);
	glEnableVertexAttribArray(position);

	glVertexAttribPointer(
		vertexUv,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(VertexData),
		(void*)offsetof(VertexData, uv)
	);
	glEnableVertexAttribArray(vertexUv);

	glVertexAttribPointer(
		normal,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(VertexData),
		(void*)offsetof(VertexData, normal)
	);
	glEnableVertexAttribArray(normal);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(textureSampler, 0);
	glUniform2f(resolution, windowWidth, windowHeight);

	buildViewProjectionMatrix();

	// Fill the screen with blue
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);

	for (auto& cubeSet : chunk.liveBlocks())
	{
		BlockLibrary::Tag blockType = cubeSet.first;
		glBindTexture(GL_TEXTURE_2D, blockLibrary->get(blockType).texture);

		for (auto& cube : cubeSet.second)
		{
			glm::mat4 model = glm::translate(glm::mat4(1.0f), cube->location.vec3());
			glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, &model[0][0]);

			if (targeted && cube->location == targetedBlock)
				glUniform1i(highlight, GL_TRUE);
			else
				glUniform1i(highlight, GL_FALSE);

			// Draw a cube
			glDrawArrays(GL_TRIANGLES, 0, 3 * 12);
		}
	}

	glDisableVertexAttribArray(position);
	glDisableVertexAttribArray(vertexUv);
	glDisableVertexAttribArray(normal);
}

// A movement of 1 pixel corresponds to a rotation of how many degrees?
float rotationSpeed = 0.5;
void GLFWCALL windowResized(int width, int height)
{
	windowWidth = width;
	windowHeight = height;
	glViewport(0, 0, width, height);

	rotationSpeed = 320.0 / width;
}

bool gravity = true;
bool jump = false;
void GLFWCALL keyCallback(int key, int action)
{
	if (key == 'G' && action == GLFW_PRESS)
		gravity = !gravity;

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

	// Light blue background
	glClearColor(0.8f, 0.8f, 1.0f, 0.0f);

	glEnable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	initialize();

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

	    bool inAir = (heightAboveBlock > 1e-4) || !chunk->isSolid(blockBelow);

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
			if (!inAir && velocity.y == 0.0)
				velocity.y = JUMP_VELOCITY;
		}

		// Flying
		if (!gravity && glfwGetKey(GLFW_KEY_SPACE) == GLFW_PRESS)
		{
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

		/*
		int oldX = camera.eye.x;
		int oldZ = camera.eye.z;
		int newX = camera.eye.x + step.x;
		int newZ = camera.eye.z + step.z;
		int oldY = int(camera.eye.y - PLAYER_HEIGHT);
		int newY = int(camera.eye.y + step.y - PLAYER_HEIGHT);

		std::cout << oldX << ", " << oldY << ", " << oldZ << " -> " << newX << ", " << newY << ", " << newZ << std::endl;
		if (oldX != newX)
		{
			if (chunk->isSolid(newX, oldY, oldZ) ||
				chunk->isSolid(newX, oldY, newZ) ||
				chunk->isSolid(newX, newY, oldZ) ||
				chunk->isSolid(newX, newY, newZ))
			{
				step.x = 0;
				velocity.x = 0;
			}
		}

		if (oldY != newY)
		{
			if (chunk->isSolid(oldX, newY, oldZ) ||
				chunk->isSolid(oldX, newY, newZ) ||
				chunk->isSolid(newX, newY, oldZ) ||
				chunk->isSolid(newX, newY, newZ))
			{
				step.y = 0;
				velocity.y = 0;
			}
		}

		if (oldZ != newZ)
		{
			if (chunk->isSolid(oldX, oldY, newZ) ||
				chunk->isSolid(oldX, newY, newZ) ||
				chunk->isSolid(newX, oldY, newZ) ||
				chunk->isSolid(newX, newY, newZ))
			{
				step.z = 0;
				velocity.z = 0;
			}
		}
		*/

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

		render(*chunk);

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

