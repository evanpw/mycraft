#include "renderer.hpp"
#include "shaders.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>

// This struct can be passed directly in a vertex buffer, but we can assign
// to it from a glm::vec3
struct raw_vec3
{
	GLfloat value[3];

	raw_vec3& operator=(const glm::vec3& that)
	{
		value[0] = that.x;
		value[1] = that.y;
		value[2] = that.z;

		return *this;
	}
};

std::ostream& operator<<(std::ostream& out, const raw_vec3& v)
{
	out << v.value[0] << ", " << v.value[1] << ", " << v.value[2];
	return out;
}

struct VertexData
{
	raw_vec3 position;
	raw_vec3 texCoord;
	raw_vec3 normal;
};

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

Renderer::Renderer(int width, int height, const World& world)
: m_world(world), m_blockLibrary(new BlockLibrary)
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
	//m_modelMatrix = glGetUniformLocation(m_programId, "modelMatrix");
	m_textureSampler = glGetUniformLocation(m_programId, "textureSampler");
	//m_highlight = glGetUniformLocation(m_programId, "highlight");
	m_resolution = glGetUniformLocation(m_programId, "resolution");
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
			vertex.position = glm::vec3(model * glm::vec4(meshVertex.position, 1.0));
			vertex.texCoord = meshVertex.position;
			vertex.normal = meshVertex.normal;

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
	// All of the blocks have the same view and projection matrices
	buildViewProjectionMatrix(camera);

	// Fill the screen with sky color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (auto& i : m_world.chunks)
	{
		const Chunk* chunk = i.second.get();
		renderChunk(chunk);
	}
}

void Renderer::renderChunk(const Chunk* chunk)
{
	const ChunkRenderingData& chunkData = getRenderingData(chunk);

	glUseProgram(m_programId);
	glBindBuffer(GL_ARRAY_BUFFER, chunkData.vertexBuffer);

	glVertexAttribPointer(m_position, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, position));
	glEnableVertexAttribArray(m_position);

	glVertexAttribPointer(m_texCoord, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, texCoord));
	glEnableVertexAttribArray(m_texCoord);

	glVertexAttribPointer(m_normal, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, normal));
	glEnableVertexAttribArray(m_normal);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(m_textureSampler, 0);
	glUniform2f(m_resolution, m_width, m_height);

	glBindTexture(GL_TEXTURE_CUBE_MAP, m_blockLibrary->get(BlockLibrary::GRASS).texture);
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