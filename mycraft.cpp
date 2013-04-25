#define GLM_SWIZZLE
#include <glm/glm.hpp>

#include "block_library.hpp"
#include "chunk.hpp"
#include "coordinate.hpp"
#include "renderer.hpp"
#include "shaders.hpp"

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
#include <glm/gtc/matrix_transform.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <tbb/concurrent_queue.h>

const int INITIAL_WIDTH = 640;
const int INITIAL_HEIGHT = 480;

void copyVector(GLfloat* dest, const glm::vec3& source)
{
	dest[0] = source.x;
	dest[1] = source.y;
	dest[2] = source.z;
}

struct CubeVertex
{
	glm::vec3 position, normal;
};

const std::array<CubeVertex, 36> cubeMesh =
{{
	// Right face
	{glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)},
	{glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f)},
	{glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)},

	{glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f)},
	{glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)},
	{glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f)},


	// Left face
	{glm::vec3(0.0f, 1.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f)},
	{glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f)},
	{glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f)},

	{glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f)},
	{glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f)},
	{glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f)},


	// Top face
	{glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)},
	{glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)},
	{glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f)},

	{glm::vec3(0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f)},
	{glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f)},
	{glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)},


	// Bottom face
	{glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)},
	{glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)},
	{glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)},

	{glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)},
	{glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)},
	{glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)},


	// Front face
	{glm::vec3(0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)},
	{glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)},
	{glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)},

	{glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)},
	{glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)},
	{glm::vec3(0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)},


	// Back face
	{glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)},
	{glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)},
	{glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)},

	{glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)},
	{glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)},
	{glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)},
}};

Renderer* renderer;

class ChunkManager
{
public:
	ChunkManager(int seed);

	std::vector<Mesh*> getVisibleMeshes(const Camera& camera);

	// Access the world
	const Block* getBlock(const Coordinate& r);
	bool isTransparent(const Coordinate& r);

	void setSize(int width, int height);

private:
	// The seed for the PRNG used by the terrain generator
	int m_seed;

	void createMesh(const Chunk* chunk);
	void freeMesh(const Chunk* chunk);

	// Gets the chunk at the given coordinates, or null if it has
	// not yet been generated/loaded.
	const Chunk* getChunk(int x, int z);

	// Returns null if the mesh has not yet been generated.
	Mesh* getMesh(const Chunk* chunk);

	std::vector<Block*> getLiveBlocks(const Chunk* chunk);
	unsigned int getLiveFaces(const Coordinate& r);

	// Chunks are loaded/generated asynchronously
	std::set<std::pair<int, int>> m_chunkQueue;
	void loadOrCreateChunk(int x, int z);

	// Twice the number of chunks in the render radius
	static const size_t MAX_OBJECTS = 200;
	std::vector<GLuint> m_vboPool;

	std::map<std::pair<int, int>, std::unique_ptr<Chunk>> m_chunks;

	// Meshes are rebuilt asynchronously
	void rebuildMesh(const Chunk* chunk);

	std::map<const Chunk*, std::unique_ptr<Mesh>> m_meshes;
};

ChunkManager::ChunkManager(int seed)
: m_seed(seed)
{
	// Create a bunch of vertex buffers initially, so that we don't
	// have to keep allocating and deleting
	m_vboPool.resize(MAX_OBJECTS);
	glGenBuffers(MAX_OBJECTS, &m_vboPool[0]);
}

void ChunkManager::createMesh(const Chunk* chunk)
{
	assert(!m_vboPool.empty());

	Mesh* mesh = new Mesh;
	mesh->vertexBuffer = m_vboPool.back();
	m_vboPool.pop_back();
	mesh->needsRebuilt = true;

	m_meshes[chunk] = std::unique_ptr<Mesh>(mesh);
}

void ChunkManager::freeMesh(const Chunk* chunk)
{
	auto i = m_meshes.find(chunk);
	if (i != m_meshes.end())
	{
		// Return the vertex buffer to the pool to be reused
		m_vboPool.push_back(i->second->vertexBuffer);
		m_meshes.erase(i);
	}
}

const Chunk* ChunkManager::getChunk(int x, int z)
{
	auto i = m_chunks.find(std::make_pair(x, z));
	if (i == m_chunks.end())
	{
		return nullptr;
	}
	else
	{
		return i->second.get();
	}
}

Mesh* ChunkManager::getMesh(const Chunk* chunk)
{
	auto i = m_meshes.find(chunk);
	if (i == m_meshes.end())
	{
		return nullptr;
	}
	else
	{
		return i->second.get();
	}
}

void ChunkManager::loadOrCreateChunk(int x, int z)
{
	// TODO: Load from a file
	std::unique_ptr<Chunk> newChunk(new Chunk(x, z, m_seed));
	m_chunks[std::make_pair(x, z)] = std::move(newChunk);
}

class DistanceToCamera
{
public:
	DistanceToCamera(const Camera& camera)
	{
		m_camera = camera.eye.xz;
	}

	static glm::vec2 chunkCenter(const std::pair<int, int>& location)
	{
		return float(Chunk::SIZE) * glm::vec2(location.first + 0.5, location.second + 0.5);
	}

	bool operator()(const std::pair<int, int>& lhs, const std::pair<int, int>& rhs)
	{
		return glm::distance(chunkCenter(lhs), m_camera) < glm::distance(chunkCenter(rhs), m_camera);
	}

private:
	glm::vec2 m_camera;
};

const size_t MAX_PER_FRAME = 1;
std::vector<Mesh*> ChunkManager::getVisibleMeshes(const Camera& camera)
{
	size_t processed = 0;
	while (!m_chunkQueue.empty() && processed < MAX_PER_FRAME)
	{
		auto i = std::min_element(m_chunkQueue.begin(), m_chunkQueue.end(), DistanceToCamera(camera));
		std::pair<int, int> chunkCoord = *i;
		m_chunkQueue.erase(i);

		if (!getChunk(chunkCoord.first, chunkCoord.second))
		{
			loadOrCreateChunk(chunkCoord.first, chunkCoord.second);
			++processed;
			if (processed >= MAX_PER_FRAME) break;
		}

		const Chunk* chunk = getChunk(chunkCoord.first, chunkCoord.second);
		if (!getMesh(chunk))
		{
			createMesh(chunk);
		}

		if (getMesh(chunk)->needsRebuilt)
		{
			rebuildMesh(chunk);
			++processed;
		}
	}

	std::vector<Mesh*> meshes;

	int x = floor(camera.eye.x / (float)Chunk::SIZE);
	int z = floor(camera.eye.z / (float)Chunk::SIZE);
	for (int i = -Renderer::RENDER_RADIUS; i <= Renderer::RENDER_RADIUS; ++i)
	{
		for (int j = -Renderer::RENDER_RADIUS; j <= Renderer::RENDER_RADIUS; ++j)
		{
			const Chunk* chunk = getChunk(x + i, z + j);
			if (!chunk)
			{
				m_chunkQueue.insert(std::make_pair(x + i, z + j));
				continue;
			}

			Mesh* mesh = getMesh(chunk);
			if (!mesh)
			{
				m_chunkQueue.insert(std::make_pair(x + i, z + j));
			}
			else
			{
				if (mesh->needsRebuilt)
				{
					m_chunkQueue.insert(std::make_pair(x + i, z + j));
				}

				// Render dirty chunks also - it's better to show a slightly out-of-date
				// image rather than a giant hole in the world.
				meshes.push_back(mesh);
			}
		}
	}

	//std::cout << "Loaded chunks: " << m_chunks.size() << ", loaded meshes = " << m_meshes.size() << std::endl;
	//std::cout << "In queue: " << m_chunkQueue.size() << std::endl;

	// Free/unload chunks and meshes that are far away from the camera
	glm::vec2 camera2d = camera.eye.xz;

	// We have to do this loop manually because elements are being deleted inside the loop
	auto j = m_chunks.begin();
	while (j != m_chunks.end())
	{
		auto current = j++;
		const std::pair<int, int>& location = current->first;
		const Chunk* chunk = current->second.get();

		glm::vec2 chunkCenter = DistanceToCamera::chunkCenter(location);
		if (glm::distance(chunkCenter, camera2d) > 5 * Renderer::RENDER_RADIUS * Chunk::SIZE)
		{
			//std::cout << "Freeing chunk at " << chunk->x() << " " << chunk->z() << std::endl;
			freeMesh(chunk);
			m_chunks.erase(current);
		}
		else if (getMesh(chunk) && glm::distance(chunkCenter, camera2d) > 2 * Renderer::RENDER_RADIUS * Chunk::SIZE)
		{
			//std::cout << "Freeing mesh at " << chunk->x() << " " << chunk->z() << std::endl;
			freeMesh(chunk);
		}
	}

	return meshes;
}

const Block* ChunkManager::getBlock(const Coordinate& location)
{
	int x = floor(location.x / float(Chunk::SIZE));
	int z = floor(location.z / float(Chunk::SIZE));

	const Chunk* chunk = getChunk(x, z);
	if (chunk)
	{
		return chunk->get(location);
	}
	else
	{
		return nullptr;
	}
}

bool ChunkManager::isTransparent(const Coordinate& location)
{
	const Block* block = getBlock(location);

	// TODO: Support transparent blocks other than air
	return (block == nullptr);
}

std::vector<Block*> ChunkManager::getLiveBlocks(const Chunk* chunk)
{
	// TODO: Pre-compute a lot of this

	std::vector<Block*> liveBlocks;
	for (auto& i : chunk->blocks())
	{
		const Coordinate& r = i.first;
		const std::unique_ptr<Block>& block = i.second;

		// Check all sides of the cube
		if (getLiveFaces(r) != 0)
			liveBlocks.push_back(block.get());
	}

	return liveBlocks;
}

const unsigned int PLUS_X = 1 << 0;
const unsigned int MINUS_X = 1 << 1;
const unsigned int PLUS_Y = 1 << 2;
const unsigned int MINUS_Y = 1 << 3;
const unsigned int PLUS_Z = 1 << 4;
const unsigned int MINUS_Z = 1 << 5;

unsigned int ChunkManager::getLiveFaces(const Coordinate& r)
{
	unsigned int mask = 0;
	if (isTransparent(r.addX(1))) mask |= PLUS_X;
	if (isTransparent(r.addX(-1))) mask |= MINUS_X;
	if (isTransparent(r.addY(1))) mask |= PLUS_Y;
	if (isTransparent(r.addY(-1))) mask |= MINUS_Y;
	if (isTransparent(r.addZ(1))) mask |= PLUS_Z;
	if (isTransparent(r.addZ(-1))) mask |= MINUS_Z;

	return mask;
}

void ChunkManager::rebuildMesh(const Chunk* chunk)
{
	Mesh* mesh = getMesh(chunk);
	assert(mesh != nullptr);

	mesh->vertices.clear();

	unsigned int masks[6] =
	{
		PLUS_X, MINUS_X,
		PLUS_Y, MINUS_Y,
		PLUS_Z, MINUS_Z
	};

	// Create the vertex data
	for (auto& cube : getLiveBlocks(chunk))
	{
		// Translate the cube mesh to the appropriate place in world coordinates
		glm::mat4 model = glm::translate(glm::mat4(1.0f), cube->location.vec3());

		unsigned int liveFaces = getLiveFaces(cube->location);
		for (size_t face = 0; face < 6; ++face)
		{
			if (liveFaces & masks[face])
			{
				for (size_t i = 0; i < 6; ++i)
				{
					CubeVertex cubeVertex = cubeMesh[face * 6 + i];

					Vertex vertex;
					copyVector(vertex.position, glm::vec3(model * glm::vec4(cubeVertex.position, 1.0)));
					copyVector(vertex.texCoord, cubeVertex.position);
					vertex.texCoord[3] = cube->blockType;
					copyVector(vertex.normal, cubeVertex.normal);

					mesh->vertices.push_back(vertex);
				}
			}
		}
	}

	/*
	std::cout << "Vertex count: " << mesh->vertices.size() << std::endl;
	std::cout << "VBO size: " << (sizeof(Vertex) * mesh->vertices.size() / (1 << 20)) << "MB" << std::endl;
	*/

	mesh->needsRebuilt = false;
	mesh->needsUploaded = true;
}

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

const float PLAYER_HEIGHT = 1.62;	// Height of eyes in blocks
const float WALKING_SPEED = 4.3;	// Blocks / s
const float FLYING_SPEED = 2.5 * WALKING_SPEED;
const float GRAVITY = 32;			// Blocks / s^2
const float AIR_RESISTANCE = 0.4;	// s^{-1} (of the player)
const float JUMP_VELOCITY = 8.4;	// Blocks / s

// Maximum distance at which one can target (and destroy / place) a block
const float MAX_TARGET_DISTANCE = 10.0f;

ChunkManager* chunkManager;

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

	} while (chunkManager->isTransparent(currentBlock));

	result = currentBlock;
	return true;
}

// A movement of 1 pixel corresponds to a rotation of how many degrees?
float rotationSpeed = 0.5;
void GLFWCALL windowResized(int width, int height)
{
	rotationSpeed = 320.0 / width;
	if (renderer)
		renderer->setSize(width, height);
}

bool gravity = false;
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
			//Coordinate targetedBlock;
			//bool targeted = castRay(targetedBlock);

			//if (targeted)
			//{
			//	const Chunk* chunk = world->removeBlock(targetedBlock);
			//	renderer->invalidate(chunk);
			//}
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

	renderer = new Renderer(INITIAL_WIDTH, INITIAL_HEIGHT);
	chunkManager = new ChunkManager(rand());

	// Start up in the air
	camera.eye = glm::vec3(0.0, 64.0, 0.0);

	int lastx, lasty;
	glfwGetMousePos(&lastx, &lasty);

	glfwEnable(GLFW_MOUSE_CURSOR);

	float lastUpdate = glfwGetTime();

	// Loop until the escape key is pressed or the window is closed
	while (glfwGetWindowParam(GLFW_OPENED))
	{
		// Time elapsed since last update
		float now = glfwGetTime();
		float elapsed = now - lastUpdate;
		lastUpdate = now;

	    // y-coordinate of the player's feet
	    float feetY = camera.eye.y - PLAYER_HEIGHT;

	    // Height of the player's feet above the block directly below
	    float heightAboveBlock = feetY - int(floor(feetY));

	    // Coordinates the block directly below the player's feet
	    Coordinate blockBelow(camera.eye.x, int(feetY) - 1, camera.eye.z);

	    bool inAir = (heightAboveBlock > 1e-3) || true; //!world->isSolid(blockBelow);
	    float speed = (!gravity && inAir) ? FLYING_SPEED : WALKING_SPEED;

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
			step.y += speed;
		}

		// Falling
		if (gravity)
		{
			if (inAir)
			{
				velocity -= elapsed * GRAVITY * glm::vec3(0.0f, 1.0f, 0.0f);
				velocity *= (1 - AIR_RESISTANCE * elapsed);
			}
		}
		else if (glfwGetKey(GLFW_KEY_LSHIFT) == GLFW_PRESS)
		{
			step.y -= speed;
		}

		if (glfwGetKey('W') == GLFW_PRESS)
			step += speed * facing;

		if (glfwGetKey('S') == GLFW_PRESS)
			step -= speed * facing;

		if (glfwGetKey('A') == GLFW_PRESS)
			step -= speed * right;

		if (glfwGetKey('D') == GLFW_PRESS)
			step += speed * right;

		// Actually do the falling / rising
		step += velocity;

		/*
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
		*/

		camera.eye += step * elapsed;

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

		std::vector<Mesh*> visibleMeshes = chunkManager->getVisibleMeshes(camera);
		renderer->renderMeshes(camera, visibleMeshes);
		glfwSwapBuffers();

		frames++;
	}

	// Close OpenGL window and terminate glfw
	running = false;
	thread.join();
	glfwTerminate();
	return 0;
}

