#include "renderer.hpp"
#include "shaders.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>

struct VertexData
{
	GLfloat position[3];
	GLfloat texCoord[4];
	GLfloat normal[3];
};

void copyVector(GLfloat* dest, const glm::vec3& source)
{
	dest[0] = source.x;
	dest[1] = source.y;
	dest[2] = source.z;
}

struct MeshVertex
{
	glm::vec3 position, normal;
};

const std::array<MeshVertex, 36> cubeMesh =
{{
	// Front face
	{glm::vec3(0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)},
	{glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)},
	{glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)},

	{glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)},
	{glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)},
	{glm::vec3(0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)},



	// Right face
	{glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)},
	{glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f)},
	{glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)},

	{glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f)},
	{glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)},
	{glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f)},



	// Back face
	{glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)},
	{glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)},
	{glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)},

	{glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)},
	{glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)},
	{glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)},



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
	{glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)}
}};

Renderer::Renderer(int width, int height, World& world)
: m_world(world), m_blockLibrary(new BlockLibrary)
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
	GLuint vertexArray;
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);

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

void Renderer::processChunk(const Chunk* chunk, ChunkRenderingData& data)
{
	glBindBuffer(GL_ARRAY_BUFFER, data.vertexBuffer);

	std::vector<VertexData> vertices;

	// Create the vertex data
	for (auto& cube : m_world.liveBlocks(chunk))
	{
		// Translate the cube mesh to the appropriate place in world coordinates
		glm::mat4 model = glm::translate(glm::mat4(1.0f), cube->location.vec3());

		for (auto& meshVertex : cubeMesh)
		{
			VertexData vertex;
			copyVector(vertex.position, glm::vec3(model * glm::vec4(meshVertex.position, 1.0)));
			copyVector(vertex.texCoord, meshVertex.position);
			vertex.texCoord[3] = cube->blockType;
			copyVector(vertex.normal, meshVertex.normal);

			vertices.push_back(vertex);
		}
	}

	std::cout << "Vertex count: " << vertices.size() << std::endl;
	std::cout << "VBO size: " << (sizeof(VertexData) * vertices.size() / (1 << 20)) << "MB" << std::endl;
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	data.vertexCount = vertices.size();
	data.dirty = false;
}

const ChunkRenderingData& Renderer::getRenderingData(const Chunk* chunk)
{
	// Create a new ChunkRenderingData instance if one doesn't already exist
	if (m_chunkData.find(chunk) == m_chunkData.end())
	{
		ChunkRenderingData data;
		glGenBuffers(1, &data.vertexBuffer);
		data.dirty = true;

		m_chunkData[chunk] = data;
	}

	// If dirty, update the buffer
	ChunkRenderingData& data = m_chunkData[chunk];
	if (data.dirty)
	{
		processChunk(chunk, data);
	}

	return data;
}

void Renderer::invalidate(const Chunk* chunk)
{
	auto i = m_chunkData.find(chunk);
	if (i != m_chunkData.end())
	{
		i->second.dirty = true;
	}
}

void Renderer::render(const Camera& camera)
{
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

	int x = floor(camera.eye.x / (float)Chunk::SIZE);
	int z = floor(camera.eye.z / (float)Chunk::SIZE);
	for (int i = -4; i <= 4; ++i)
	{
		for (int j = -4; j <= 4; ++j)
		{
			const Chunk* chunk = m_world.chunkAt(x + i, z + j);
			renderChunk(chunk);
		}
	}
}

void Renderer::renderChunk(const Chunk* chunk)
{
	const ChunkRenderingData& chunkData = getRenderingData(chunk);

	glUseProgram(m_programId);
	glBindBuffer(GL_ARRAY_BUFFER, chunkData.vertexBuffer);

	glVertexAttribPointer(m_position, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, position));
	glEnableVertexAttribArray(m_position);

	glVertexAttribPointer(m_texCoord, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, texCoord));
	glEnableVertexAttribArray(m_texCoord);

	glVertexAttribPointer(m_normal, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, normal));
	glEnableVertexAttribArray(m_normal);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(m_textureSampler, 0);
	glUniform2f(m_resolution, m_width, m_height);

	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, m_blockLibrary->getTextureArray());
	glDrawArrays(GL_TRIANGLES, 0, chunkData.vertexCount);

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