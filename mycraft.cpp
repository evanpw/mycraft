#include <cstdio>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <GL/glew.h>
#include <GL/glfw.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <png.h>

uint32_t* readPng(const char* fileName, int* outWidth, int* outHeight)
{
    png_structp png_ptr;
    png_infop info_ptr;
    FILE *fp;

    if ((fp = fopen(fileName, "rb")) == nullptr)
    {
    	std::cerr << "readPng: Unable to open file: " << fileName << std::endl;
        return nullptr;
    }

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (png_ptr == nullptr)
    {
    	std::cerr << "readPng: Unable to create read_struct" << std::endl;
        fclose(fp);
        return nullptr;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == nullptr)
    {
    	std::cerr << "readPng: Unable to create info struct" << std::endl;
        fclose(fp);
        png_destroy_read_struct(&png_ptr, nullptr, nullptr);
        return nullptr;
    }


    if (setjmp(png_jmpbuf(png_ptr)))
    {
    	std::cerr << "Error calling setjmp" << std::endl;
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        fclose(fp);
        return false;
    }

    png_init_io(png_ptr, fp);

    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_EXPAND, nullptr);

    png_uint_32 width = png_get_image_width(png_ptr, info_ptr);
    png_uint_32 height = png_get_image_height(png_ptr, info_ptr);

    png_uint_32 bitdepth   = png_get_bit_depth(png_ptr, info_ptr);
    png_uint_32 channels   = png_get_channels(png_ptr, info_ptr);
    png_uint_32 color_type = png_get_color_type(png_ptr, info_ptr);

    std::cout << "Bit depth: " << bitdepth << std::endl;
    std::cout << "Channels: " << channels << std::endl;
    std::cout << "Color type: " << color_type << std::endl;

    *outWidth = width;
    *outHeight = height;

    png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);

    static_assert(sizeof(uint32_t) == 4, "uint32_t not 32 bits");

    uint32_t* data = new uint32_t[width * height];
    for (size_t i = 0; i < height; ++i)
    {
    	// Invert the y-coordinate
        memcpy(&data[i * width], row_pointers[height -1 - i], width * sizeof(uint32_t));
    }

    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    fclose(fp);

    return data;
}

enum CubeType { TREE = 0, STONE = 1};
const size_t BLOCK_TYPES = 2;
GLuint textures[BLOCK_TYPES];

bool makeTexture(const char* filename, CubeType type)
{
    int width, height;
    uint32_t* pixels = readPng(filename, &width, &height);
    if (pixels == 0)
    {
    	std::cerr << "Problem loading texture " << filename << std::endl;
    	return false;
    }
    
    glBindTexture(GL_TEXTURE_2D, textures[type]);

    glTexImage2D(
        GL_TEXTURE_2D,				// Target
        0,           				// Level of detail
        GL_RGBA8,                   // Internal format
        width, height, 0,           // Width, Height, and Border
        GL_RGBA, GL_UNSIGNED_BYTE,  // External format, type
        pixels                      // Pixel data
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);

    delete[] pixels;
    return true;
}

void readFile(const char* fileName, std::string& buffer)
{
	std::ifstream f(fileName);
	std::stringstream ss;
	ss << f.rdbuf();

	buffer = ss.str();
}

GLuint loadShader(const char* fileName, GLenum shaderType)
{
	GLuint shaderId = glCreateShader(shaderType);

	std::string code;
	readFile(fileName, code);
	if (code.size() == 0)
	{
		std::cerr << "Unable to read shader: " << fileName << std::endl;
		return 0;
	}     

	std::cout << "Compiling shader: " << fileName << std::endl;
	const char* codePointer = code.c_str();
	glShaderSource(shaderId, 1, &codePointer, nullptr);
	glCompileShader(shaderId);

	// Check the shader
	GLint result = GL_FALSE;
	int logLength;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
	glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0)
	{
		char* errorMessage = new char[logLength];
		glGetShaderInfoLog(shaderId, logLength, nullptr, errorMessage);
		std::cout << errorMessage << std::endl;

		delete[] errorMessage;
	}

	return shaderId;
}

GLuint linkShaders(GLuint vertexShader, GLuint fragmentShader)
{
	std::cout << "Linking shaders" << std::endl;
	GLuint programId = glCreateProgram();
	glAttachShader(programId, vertexShader);
	glAttachShader(programId, fragmentShader);
	glLinkProgram(programId);

	// Check the linked program
	GLint result;
	int logLength;
	glGetProgramiv(programId, GL_LINK_STATUS, &result);
	glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0)
	{
		char* errorMessage = new char[logLength];
		glGetProgramInfoLog(programId, logLength, nullptr, errorMessage);
		std::cerr << errorMessage << std::endl;

		delete[] errorMessage;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return programId;
}

struct VertexData
{
	GLfloat position[3];
	GLfloat uv[2];
	GLfloat normal[3];
};

// First three elements of each sub-array are vertex position in model coordinates,
// and the last two are the texture coordinates
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
GLint position, modelMatrix, vpMatrix, vertexUv, normal, textureSampler;
GLuint programId;

struct Coordinate
{
	Coordinate(int x, int y, int z)
	: x(x), y(y), z(z)
	{}

	glm::vec3 vec3()
	{
		return glm::vec3(x, y, z);
	}

	int x, y, z;
};

struct Cube
{
	Cube(const Coordinate& location, CubeType cubeType)
	: location(location), cubeType(cubeType)
	{}

	Coordinate location;
	CubeType cubeType;
};

std::vector<Cube> cubes;

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
		int scale = 1 << k;
		int d = range / 4;
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

	int fuzz(unsigned int range)
	{
		return (rand() % (2 * range + 1)) - range;
	}

	int averageCorners(size_t i, size_t j, int scale)
	{
		return (m_grid[i - scale][j - scale] +
				m_grid[i + scale][j - scale] +
				m_grid[i - scale][j + scale] +
				m_grid[i + scale][j - scale]) / 4;
	}

	int averageNeighbors(size_t i, size_t j, int scale)
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

const uint8_t CHUNK_BITS = 8;
const uint32_t CHUNK_SIZE = 1 << CHUNK_BITS;
Noise terrainHeight(CHUNK_BITS, (1 << CHUNK_BITS) / 4);

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

	for (int i = 0; i < CHUNK_SIZE; ++i)
	{
		for (int j = 0; j < CHUNK_SIZE; ++j)
		{
			// Stone floor
			uint8_t top = (uint8_t)terrainHeight(i, j);
			for (int y = 0; y < top; ++y)
			{
				cubes.push_back(Cube(Coordinate(i, y, j), STONE));
			}

			// Trees
			if (rand() % 256 == 0)
			{
				int treeHeight = 4 + (rand() % 3);
				for (int k = 0; k < treeHeight; ++k)
				{
					cubes.push_back(Cube(Coordinate(i, top + k, j), TREE));
				}
			}
		}
	}

	// Start up in the air
	float location = (CHUNK_SIZE / 2.0) + 0.5;
	camera.eye = glm::vec3(location, 50.0, location);

	// Load the texture
	glGenTextures(BLOCK_TYPES, textures);
	makeTexture("tree.png", TREE);
	makeTexture("stone.png", STONE);
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

bool operator<(const Coordinate& lhs, const Coordinate& rhs)
{
	if (lhs.x < rhs.x) return true;
	else if (lhs.x > rhs.x) return false;

	if (lhs.y < rhs.y) return true;
	else if (lhs.y > rhs.y) return false;

	if (lhs.z < rhs.z) return true;
	else return false;
}

std::vector<Cube*> getActiveCubes()
{
	std::map<Coordinate, Cube*> allCubes;
	for (auto& cube : cubes)
	{
		Coordinate r(cube.location.x, cube.location.y, cube.location.z);
		allCubes[r] = &cube;
	}

	std::cout << "Total cubes: " << allCubes.size() << std::endl;

	std::vector<Cube*> activeCubes;
	for (auto& i : allCubes)
	{
		const Coordinate r = i.first;
		Cube* cube = i.second;

		// Check all sides of the cube
		if (allCubes.find(Coordinate(r.x + 1, r.y, r.z)) == allCubes.end() ||
			allCubes.find(Coordinate(r.x - 1, r.y, r.z)) == allCubes.end() ||
			allCubes.find(Coordinate(r.x, r.y + 1, r.z)) == allCubes.end() ||
			allCubes.find(Coordinate(r.x, r.y - 1, r.z)) == allCubes.end() ||
			allCubes.find(Coordinate(r.x, r.y, r.z + 1)) == allCubes.end() ||
			allCubes.find(Coordinate(r.x, r.y, r.z - 1)) == allCubes.end())
		{
			activeCubes.push_back(cube);
		}
	}

	std::cout << "Active cubes: " << activeCubes.size() << std::endl;

	return activeCubes;
}

void render(const std::vector<Cube*> activeCubes)
{
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

	buildViewProjectionMatrix();

	// Fill the screen with blue
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);

	for (auto& cube : activeCubes)
	{
		glBindTexture(GL_TEXTURE_2D, textures[cube->cubeType]);
		glm::mat4 model = glm::translate(glm::mat4(1.0f), cube->location.vec3());
		glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, &model[0][0]);

		// Draw a cube
		glDrawArrays(GL_TRIANGLES, 0, 3 * 12);
	}
	
	glDisableVertexAttribArray(position);
	glDisableVertexAttribArray(vertexUv);
	glDisableVertexAttribArray(normal);
}

// Minecraft's value
//const float PLAYER_HEIGHT = 1.62;	// Height of eyes in blocks

// The real-world height of my eyes
const float PLAYER_HEIGHT = 1.8161;	// Height of eyes in blocks

const float WALKING_SPEED = 4.3;	// Blocks / s
const float FLYING_SPEED = 1.5 * WALKING_SPEED;
const float GRAVITY = 32;			// Blocks / s^2
const float AIR_RESISTANCE = 0.4;	// s^{-1} (of the player)
const float JUMP_VELOCITY = 8.4;	// Blocks / s

// A movement of 1 pixel corresponds to a rotation of how many degrees?
float rotationSpeed = 1.0;
void GLFWCALL windowResized(int width, int height)
{
	windowWidth = width;
	windowHeight = height;
	glViewport(0, 0, width, height);

	rotationSpeed = 640.0 / width;
}

bool gravity = true;
bool jump = false;
void GLFWCALL keyCallback(int key, int action)
{
	if (key == 'G' && action == GLFW_PRESS)
		gravity = !gravity;
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

	bool mouseCaptured = false;
	glfwEnable(GLFW_MOUSE_CURSOR);

	float lastTime = glfwGetTime();
	float lastFrame = 0.0f;
 	int nbFrames = 0;

 	std::vector<Cube*> activeCubes = getActiveCubes();

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

		if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{
			mouseCaptured = true;
			glfwDisable(GLFW_MOUSE_CURSOR);
		}

		if (glfwGetKey(GLFW_KEY_ESC) == GLFW_PRESS)
		{
			mouseCaptured = false;
			glfwEnable(GLFW_MOUSE_CURSOR);
		}

		// Falling and rising
		int i = camera.eye.x;
		int j = camera.eye.z;
		int height;
		if (i >= 0 && i < CHUNK_SIZE && j >= 0 && j < CHUNK_SIZE)
			height = terrainHeight(i, j);
		else
			height = -1000;

		float blocksPerFrame = WALKING_SPEED * lastFrame;
		if (!gravity && camera.eye.y - (height + PLAYER_HEIGHT) > 1e-3)
		{
			blocksPerFrame = FLYING_SPEED * lastFrame;
		}

		glm::mat4 rotation = glm::rotate(glm::mat4(1.0), camera.horizontalAngle, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec3 facing = glm::mat3(rotation) * glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 right = glm::cross(facing, glm::vec3(0.0f, 1.0f, 0.0f));

		if (glfwGetKey('W') == GLFW_PRESS)
			camera.eye += (blocksPerFrame * facing);

		if (glfwGetKey('S') == GLFW_PRESS)
			camera.eye -= (blocksPerFrame * facing);

		if (glfwGetKey('A') == GLFW_PRESS)
			camera.eye -= (blocksPerFrame * right);

		if (glfwGetKey('D') == GLFW_PRESS)
			camera.eye += (blocksPerFrame * right);

		if (glfwGetKey('R') == GLFW_PRESS && gravity && (camera.eye.y - (height + PLAYER_HEIGHT) < 1e-4))
		{
			velocity.y += 10 * JUMP_VELOCITY;
		}

		// Jumping / Flying
		if (glfwGetKey(GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			if (gravity && (camera.eye.y - (height + PLAYER_HEIGHT) < 1e-4))
			{
				velocity.y += JUMP_VELOCITY;
			}
			else if (!gravity)
			{
				camera.eye.y += blocksPerFrame;
			}
		}

		// Falling
		if (gravity)
		{
			if (camera.eye.y > height + PLAYER_HEIGHT)
			{
				velocity -= GRAVITY * lastFrame * glm::vec3(0.0f, 1.0f, 0.0f);
				velocity *= (1 - AIR_RESISTANCE * lastFrame);
			}
		}
		else if (glfwGetKey(GLFW_KEY_LSHIFT) == GLFW_PRESS)
		{
			camera.eye.y -= blocksPerFrame;
		}
		
		// Actually do the falling / rising
		camera.eye += velocity * lastFrame;

		// Stop falling once the ground is hit
		if (camera.eye.y < height + PLAYER_HEIGHT)
		{
			camera.eye.y = height + PLAYER_HEIGHT;
			velocity.y = 0;
		}

		int x, y;
		glfwGetMousePos(&x, &y);

		if (mouseCaptured)
		{
			camera.horizontalAngle -= rotationSpeed * (x - lastx);
			camera.verticalAngle -= rotationSpeed * (y - lasty);

			if (camera.verticalAngle < -90.0) camera.verticalAngle = -90.0;
			if (camera.verticalAngle > 90.0) camera.verticalAngle = 90.0;
		}

		lastx = x;
		lasty = y;

		render(activeCubes);

		// Display on the screen
		glfwSwapBuffers();
		nbFrames++;
		lastFrame = glfwGetTime() - currentTime;
	}

	// Close OpenGL window and terminate glfw
	glfwTerminate();
	return 0;
}

