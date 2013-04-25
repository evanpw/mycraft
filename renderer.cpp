#define GLM_SWIZZLE
#include <glm/glm.hpp>

#include "renderer.hpp"
#include "shaders.hpp"
#include <array>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <gl/glfw.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Renderer::Renderer(int width, int height)
: m_blockLibrary(new BlockLibrary)
{
	setSize(width, height);

	// Cull back faces
	glEnable(GL_CULL_FACE);

	// We don't sort blocks ourselves, so we need depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// For transparent blocks
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Create a vertex array object
	glGenVertexArrays(1, &m_vertexArray);
	glBindVertexArray(m_vertexArray);

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
	m_textureSampler = glGetUniformLocation(m_programId, "textureSampler");
	//m_highlight = glGetUniformLocation(m_programId, "highlight");
	m_resolution = glGetUniformLocation(m_programId, "resolution");
	m_sunPosition = glGetUniformLocation(m_programId, "sunPosition");
	m_brightness = glGetUniformLocation(m_programId, "brightness");
}

Renderer::~Renderer()
{
	glDeleteVertexArrays(1, &m_vertexArray);
	glDeleteProgram(m_programId);
}

void Renderer::uploadMesh(Mesh* mesh)
{
	assert(mesh->needsUploaded);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * mesh->vertices.size(), &mesh->vertices[0], GL_STATIC_DRAW);

	// We no longer need to store the mesh vertex data in main memory
	mesh->vertexCount = mesh->vertices.size();
	mesh->vertices.clear();
	mesh->vertices.shrink_to_fit();
	mesh->needsUploaded = false;
}

void Renderer::renderMeshes(const Camera& camera, const std::vector<Mesh*>& meshes)
{
	static glm::vec3 sun(-4.0, 2.0, 1.0);
	//glm::mat4 rotation = glm::rotate(glm::mat4(1.0), 0.1f, glm::vec3(1.0f, 0.0f, 0.0f));
	//sun = glm::vec3(rotation * glm::vec4(sun, 1.0));

	// All of the blocks have the same view and projection matrices
	buildViewProjectionMatrix(camera);

	// Adjust the brighness level depending on the height of the sun
	float brightness = 1.0;
	float sunHeight = sun.y / sqrt(sun.y * sun.y + sun.z * sun.z);
	if (sunHeight < 0.2)
		brightness = glm::clamp(sunHeight + 0.8, 0.0, 1.0);
	glUniform1f(m_brightness, brightness);

	// Fill the screen with sky color
	glm::vec3 skyColor = brightness * glm::vec3(0.6f, 0.6f, 1.0f);
	glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUniform3fv(m_sunPosition, 1, &sun[0]);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(m_textureSampler, 0);
	glUniform2f(m_resolution, m_width, m_height);

	glUseProgram(m_programId);

	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, m_blockLibrary->getTextureArray());

	glEnableVertexAttribArray(m_position);
	glEnableVertexAttribArray(m_texCoord);
	glEnableVertexAttribArray(m_normal);

	size_t uploaded = 0;
	for (Mesh* mesh : meshes)
	{
		if (mesh->needsUploaded)
		{
			uploadMesh(mesh);
			++uploaded;
		}

		glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
		glVertexAttribPointer(m_position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
		glVertexAttribPointer(m_texCoord, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
		glVertexAttribPointer(m_normal, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

		glDrawArrays(GL_TRIANGLES, 0, mesh->vertexCount);
	}

	// Good OpenGL hygiene
	glDisableVertexAttribArray(m_position);
	glDisableVertexAttribArray(m_texCoord);
	glDisableVertexAttribArray(m_normal);
}

/*
void Renderer::processChunk(const Chunk* chunk, ChunkRenderingData* data)
{
	float start = glfwGetTime();

	std::vector<Vertex> vertices;
	vertices.reserve(m_world.liveBlocks(chunk).size() * 36);

	// Create the vertex data
	for (auto& cube : m_world.liveBlocks(chunk))
	{
		// Translate the cube mesh to the appropriate place in world coordinates
		glm::mat4 model = glm::translate(glm::mat4(1.0f), cube->location.vec3());

		for (auto& meshVertex : cubeMesh)
		{
			Vertex vertex;
			copyVector(vertex.position, glm::vec3(model * glm::vec4(meshVertex.position, 1.0)));
			copyVector(vertex.texCoord, meshVertex.position);
			vertex.texCoord[3] = cube->blockType;
			copyVector(vertex.normal, meshVertex.normal);

			vertices.push_back(vertex);
		}
	}

	float generated = glfwGetTime();

	std::cout << "Vertex count: " << vertices.size() << std::endl;
	std::cout << "VBO size: " << (sizeof(Vertex) * vertices.size() / (1 << 20)) << "MB" << std::endl;

	glBindBuffer(GL_ARRAY_BUFFER, data->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	float uploaded = glfwGetTime();

	std::cout << "Time to generate geometry: " << (generated - start) << ", time to upload geometry: " << (uploaded - generated) << std::endl;

	data->vertexCount = vertices.size();
	data->dirty = false;
}

ChunkRenderingData* Renderer::getRenderingData(const Chunk* chunk)
{
	auto i = m_chunkData.find(chunk);
	if (i != m_chunkData.end())
	{
		return i->second.get();
	}
	else
	{
		return nullptr;
	}
}

void Renderer::invalidate(const Chunk* chunk)
{
	auto i = m_chunkData.find(chunk);
	if (i != m_chunkData.end())
	{
		i->second->dirty = true;
	}
}

// Normalize an angle into [0, 2pi)
float normalizeAngle(float angle)
{
	while (angle < 0) angle += 2 * M_PI;
	while (angle > 2 * M_PI) angle -= 2 * M_PI;

	return angle;
}

// Determine if two arcs of a circle intersect. All parameters are in radians,
// but they may not be normalized
bool arcsIntersect(float lower1, float upper1, float lower2, float upper2)
{
	lower1 = normalizeAngle(lower1);
	upper1 = normalizeAngle(upper1);
	lower2 = normalizeAngle(lower2);
	upper2 = normalizeAngle(upper2);

	if (lower1 > upper1) lower1 -= 2 * M_PI;
	if (lower2 > upper2) lower2 -= 2 * M_PI;

	std::cout << "arcsIntersect: " << lower1 << " " << upper1 << " " << lower2 << " " << upper2 << std::endl;

	return !((upper1 < lower2) || (upper2 < lower1));
}


const int RENDER_RADIUS = 4;

void Renderer::findFrustum(const Camera& camera)
{
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0), camera.horizontalAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	rotation = glm::rotate(rotation, camera.verticalAngle, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::vec3 gaze = glm::normalize(glm::mat3(rotation) * glm::vec3(0.0f, 0.0f, -1.0f));
	glm::vec3 up = glm::normalize(glm::mat3(rotation) * glm::vec3(0.0, 1.0f, 0.0f));
	glm::vec3 right = glm::cross(up, gaze);

	float halfHeight = tan((M_PI / 4.0) * 0.5);
	float halfWidth = (halfHeight * m_height) / m_width;

	glm::vec3 corner0 = gaze - (halfWidth * right) + (halfHeight * up);
	glm::vec3 corner1 = gaze + (halfWidth * right) + (halfHeight * up);
	glm::vec3 corner2 = gaze + (halfWidth * right) - (halfHeight * up);
	glm::vec3 corner3 = gaze - (halfWidth * right) - (halfHeight * up);
}

class DistanceToCamera
{
public:
	DistanceToCamera(const Camera& camera)
	{
		m_camera = camera.eye.xz;
	}

	bool operator()(const std::pair<int, int>& lhs, const std::pair<int, int>& rhs)
	{
		glm::vec2 lhsCenter = float(Chunk::SIZE) * glm::vec2(lhs.first + 0.5, lhs.second + 0.5);
		glm::vec2 rhsCenter = float(Chunk::SIZE) * glm::vec2(rhs.first + 0.5, rhs.second + 0.5);

		return glm::distance(lhsCenter, m_camera) < glm::distance(rhsCenter, m_camera);
	}

private:
	glm::vec2 m_camera;
};

void Renderer::render(const Camera& camera)
{
	// Update up to one chunk per frame
	if (!m_queue.empty())
	{
		// Pop the nearest chunk from the queue
		auto i = std::min_element(m_queue.begin(), m_queue.end(), DistanceToCamera(camera));
		std::pair<int, int> coordinate = *i;
		m_queue.erase(i);

		int x = coordinate.first;
		int z = coordinate.second;

		// First, get the chunk, creating if necessary (this can take some time)
		const Chunk* chunk = m_world.chunkAt(x, z);

		ChunkRenderingData* data = getRenderingData(chunk);
		if (!data)
		{
			// Then, generate the vertex buffer for the chunk (this also may take some time)
			data = new ChunkRenderingData;
			std::unique_ptr<ChunkRenderingData> ptr(data);

			glGenBuffers(1, &data->vertexBuffer);
			m_chunkData[chunk] = std::move(ptr);
		}

		processChunk(chunk, data);
	}

	static glm::vec3 sun(-4.0, 2.0, 1.0);
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0), 0.1f, glm::vec3(1.0f, 0.0f, 0.0f));
	//sun = glm::vec3(rotation * glm::vec4(sun, 1.0));

	// All of the blocks have the same view and projection matrices
	buildViewProjectionMatrix(camera);

	// Adjust the brighness level depending on the height of the sun
	float brightness = 1.0;
	float sunHeight = sun.y / sqrt(sun.y * sun.y + sun.z * sun.z);
	if (sunHeight < 0.2)
		brightness = glm::clamp(sunHeight + 0.8, 0.0, 1.0);
	glUniform1f(m_brightness, brightness);

	// Fill the screen with sky color
	glm::vec3 skyColor = brightness * glm::vec3(0.6f, 0.6f, 1.0f);
	glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUniform3fv(m_sunPosition, 1, &sun[0]);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(m_textureSampler, 0);
	glUniform2f(m_resolution, m_width, m_height);

	glUseProgram(m_programId);

	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, m_blockLibrary->getTextureArray());

	int x = floor(camera.eye.x / (float)Chunk::SIZE);
	int z = floor(camera.eye.z / (float)Chunk::SIZE);
	for (int i = -RENDER_RADIUS; i <= RENDER_RADIUS; ++i)
	{
		for (int j = -RENDER_RADIUS; j <= RENDER_RADIUS; ++j)
		{
			const Chunk* chunk = m_world.getChunk(x + i, z + j);
			const ChunkRenderingData* chunkData = getRenderingData(chunk);

			// Render dirty chunks also - it's better to show a slightly out-of-date
			// image rather than a giant hole in the world.
			if (chunkData)
			{
				renderChunk(chunk, chunkData);
			}

			if (!chunkData || chunkData->dirty)
			{
				m_queue.insert(std::make_pair(x + i, z + j));
			}
		}
	}
}

void Renderer::renderChunk(const Chunk* chunk, const ChunkRenderingData* chunkData)
{
	glBindBuffer(GL_ARRAY_BUFFER, chunkData->vertexBuffer);

	glVertexAttribPointer(m_position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(m_position);

	glVertexAttribPointer(m_texCoord, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
	glEnableVertexAttribArray(m_texCoord);

	glVertexAttribPointer(m_normal, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(m_normal);

	glDrawArrays(GL_TRIANGLES, 0, chunkData->vertexCount);

	// Good OpenGL hygiene
	glDisableVertexAttribArray(m_position);
	glDisableVertexAttribArray(m_texCoord);
	glDisableVertexAttribArray(m_normal);
}
*/

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