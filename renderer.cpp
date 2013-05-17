#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "cube.hpp"
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

	// We don't sort blocks ourselves, so we need depth testing
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	// For transparent blocks
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Create a vertex array object
	glGenVertexArrays(1, &m_vertexArray);
	glBindVertexArray(m_vertexArray);

	//// Setup the terrain chunk shader program
	GLuint vertexShader = loadShader("chunk-vertex.glsl", GL_VERTEX_SHADER);
	GLuint fragmentShader = loadShader("chunk-fragment.glsl", GL_FRAGMENT_SHADER);
	m_chunkShader.programId = linkShaders(vertexShader, fragmentShader);

	// Input variables
	m_chunkShader.position = glGetAttribLocation(m_chunkShader.programId, "position");
	m_chunkShader.texCoord = glGetAttribLocation(m_chunkShader.programId, "texCoord");
	m_chunkShader.lighting = glGetAttribLocation(m_chunkShader.programId, "lighting");

	// Uniform variables
	m_chunkShader.vpMatrix = glGetUniformLocation(m_chunkShader.programId, "vpMatrix");
	m_chunkShader.textureSampler = glGetUniformLocation(m_chunkShader.programId, "textureSampler");
	//m_chunkShader.highlight = glGetUniformLocation(m_chunkShader.programId, "highlight");
	m_chunkShader.resolution = glGetUniformLocation(m_chunkShader.programId, "resolution");
	m_chunkShader.sunPosition = glGetUniformLocation(m_chunkShader.programId, "sunPosition");
	m_chunkShader.brightness = glGetUniformLocation(m_chunkShader.programId, "brightness");



	//// Setup the screen tinting shader program
	vertexShader = loadShader("tint-vertex.glsl", GL_VERTEX_SHADER);
	fragmentShader = loadShader("tint-fragment.glsl", GL_FRAGMENT_SHADER);
	m_tintShader.programId = linkShaders(vertexShader, fragmentShader);

	m_tintShader.position = glGetAttribLocation(m_tintShader.programId, "position");
	m_tintShader.color = glGetUniformLocation(m_tintShader.programId, "color");

	GLfloat tintVertices[][2] =
	{
		{-1.0f, 1.0f}, {1.0f, -1.0f}, {1.0f, 1.0f},
		{-1.0f, -1.0f}, {1.0f, -1.0f}, {-1.0f, 1.0f}
	};
	glGenBuffers(1, &m_tintShader.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_tintShader.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 12, &tintVertices[0], GL_STATIC_DRAW);



	/// Setup the block selection shader program
	vertexShader = loadShader("floating-vertex.glsl", GL_VERTEX_SHADER);
	fragmentShader = loadShader("floating-fragment.glsl", GL_FRAGMENT_SHADER);
	m_blockShader.programId = linkShaders(vertexShader, fragmentShader);

	std::vector<GLfloat> blockVertices;
	for (CubeVertex vertex : cubeMesh)
	{
		glm::vec3 position = vertex.position;

		// Center at the origin
		position -= glm::vec3(0.5f);

		glm::vec3 texCoords = position;

		// Rotate into a pleasing orientation
		position = glm::rotateY(position, 45.0f);
		position = glm::rotateX(position, 15.0f);

		// Don't take up the entire screen
		position.z -= 10.0f;

		for (size_t i = 0; i < 3; ++i)
			blockVertices.push_back(position[i]);

		for (size_t i = 0; i < 3; ++i)
			blockVertices.push_back(texCoords[i]);
	}

	m_blockShader.position = glGetAttribLocation(m_blockShader.programId, "position");
	m_blockShader.texCoord = glGetAttribLocation(m_blockShader.programId, "texCoord");
	m_blockShader.textureSampler = glGetUniformLocation(m_blockShader.programId, "textureSampler");
	m_blockShader.projection = glGetUniformLocation(m_blockShader.programId, "projection");
	m_blockShader.blockType = glGetUniformLocation(m_blockShader.programId, "blockType");

	glGenBuffers(1, &m_blockShader.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_blockShader.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 36, &blockVertices[0], GL_STATIC_DRAW);
}

Renderer::~Renderer()
{
	glDeleteVertexArrays(1, &m_vertexArray);

	glDeleteProgram(m_chunkShader.programId);

	glDeleteProgram(m_tintShader.programId);
	glDeleteBuffers(1, &m_tintShader.vbo);
}

void Renderer::render(
		const Camera& camera,
		const std::vector<const Mesh*>& meshes,
		bool underwater,
		BlockLibrary::Tag selected)
{
	glUseProgram(m_chunkShader.programId);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

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
	glUniform1f(m_chunkShader.brightness, brightness);

	// Fill the screen with sky color
	glm::vec3 skyColor = brightness * glm::vec3(0.6f, 0.6f, 1.0f);
	glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUniform3fv(m_chunkShader.sunPosition, 1, &sun[0]);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(m_chunkShader.textureSampler, 0);
	glUniform2f(m_chunkShader.resolution, m_width, m_height);

	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, m_blockLibrary->getTextureArray());

	glEnableVertexAttribArray(m_chunkShader.position);
	glEnableVertexAttribArray(m_chunkShader.texCoord);
	glEnableVertexAttribArray(m_chunkShader.lighting);

	// Pass 1 - opaque blocks, front to back
	glCullFace(GL_BACK);
	for (const Mesh* mesh : meshes)
	{
		glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
		glVertexAttribPointer(m_chunkShader.position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
		glVertexAttribPointer(m_chunkShader.texCoord, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
		glVertexAttribPointer(m_chunkShader.lighting, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, lighting));

		glDrawArrays(GL_TRIANGLES, 0, mesh->opaqueVertices);
	}

	// Pass 2 - transparent blocks, back to front
	if (underwater)
		glCullFace(GL_FRONT);
	for (auto i = meshes.rbegin(); i != meshes.rend(); ++i)
	{
		const Mesh* mesh = *i;

		glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
		glVertexAttribPointer(m_chunkShader.position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
		glVertexAttribPointer(m_chunkShader.texCoord, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
		glVertexAttribPointer(m_chunkShader.lighting, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, lighting));

		glDrawArrays(GL_TRIANGLES, mesh->opaqueVertices, mesh->transparentVertices);
	}

	// Good OpenGL hygiene
	glDisableVertexAttribArray(m_chunkShader.position);
	glDisableVertexAttribArray(m_chunkShader.texCoord);
	glDisableVertexAttribArray(m_chunkShader.lighting);

	if (underwater)
		tintScreen(glm::vec3(0.0f, 0.0f, 1.0f));

	drawBlock(selected);
}

void Renderer::tintScreen(const glm::vec3& color)
{
	glUseProgram(m_tintShader.programId);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glUniform3fv(m_tintShader.color, 1, glm::value_ptr(color));

	glBindBuffer(GL_ARRAY_BUFFER, m_tintShader.vbo);
	glEnableVertexAttribArray(m_tintShader.position);
	glVertexAttribPointer(m_tintShader.position, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(m_tintShader.position);
}

void Renderer::drawBlock(BlockLibrary::Tag blockType)
{
	glUseProgram(m_blockShader.programId);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(m_blockShader.textureSampler, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, m_blockLibrary->getTextureArray());

	glUniform1i(m_blockShader.blockType, blockType);
	glUniformMatrix4fv(m_blockShader.projection, 1, GL_FALSE, &m_projection[0][0]);

	glBindBuffer(GL_ARRAY_BUFFER, m_blockShader.vbo);
	glEnableVertexAttribArray(m_blockShader.position);
	glEnableVertexAttribArray(m_blockShader.texCoord);
	glVertexAttribPointer(m_blockShader.position, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
	glVertexAttribPointer(m_blockShader.texCoord, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

	glDrawArrays(GL_TRIANGLES, 0, 36);

	glDisableVertexAttribArray(m_blockShader.position);
	glDisableVertexAttribArray(m_blockShader.texCoord);
}

void Renderer::setSize(int width, int height)
{
	m_width = width;
	m_height = height;
	glViewport(0, 0, m_width, m_height);

	float aspectRatio = float(width) / height;
	m_projection = glm::perspective(
		45.0f,			// Field of view
		aspectRatio,
		0.1f,			// Near clipping plane
		256.0f			// Far clipping plane
	);
}

void Renderer::buildViewProjectionMatrix(const Camera& camera) const
{
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0), camera.horizontalAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	rotation = glm::rotate(rotation, camera.verticalAngle, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::vec3 gaze = glm::mat3(rotation) * glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 up = glm::mat3(rotation) * glm::vec3(0.0, 1.0f, 0.0f);

	glm::mat4 view = glm::lookAt(camera.eye, camera.eye + gaze, up);

	glm::mat4 mvp = m_projection * view;
	glUniformMatrix4fv(m_chunkShader.vpMatrix, 1, GL_FALSE, &mvp[0][0]);
}