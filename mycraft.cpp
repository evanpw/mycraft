#include <cstdio>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
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

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(
        GL_TEXTURE_2D,				// Target
        0,           				// Level of detail
        GL_RGBA8,                   // Internal format
        width, height, 0,           // Width, Height, and Border
        GL_RGBA, GL_UNSIGNED_BYTE,  // External format, type
        pixels                      // Pixel data
    );

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
GLint position, modelMatrix, mvpMatrix, vertexUv, normal, textureSampler;
GLuint programId;

struct Cube
{
	Cube(const glm::vec3& location, CubeType cubeType)
	: location(location), cubeType(cubeType)
	{}

	glm::vec3 location;
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

// Return a random number between 0 and 1
double random()
{
	return static_cast<double>(rand()) / RAND_MAX;
}

int highestPoint[10][10];

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
	mvpMatrix = glGetUniformLocation(programId, "mvpMatrix");
	modelMatrix = glGetUniformLocation(programId, "modelMatrix");
	textureSampler = glGetUniformLocation(programId, "textureSampler");

	// Build the world
	double roughHeight[10][10];
	for (int i = 0; i < 10; ++i)
	{
		for (int j = 0; j < 10; ++j)
		{
			roughHeight[i][j] = 1 + static_cast<int>(random() * 8);
		}
	}

	double smoothHeight[10][10];
	for (int iteration = 0; iteration < 3; ++iteration)
	{
		// Smooth the mountains
		for (int i = 0; i < 10; ++i)
		{
			for (int j = 0; j < 10; ++j)
			{
				double total = roughHeight[i][j];
				unsigned int samples = 1;

				if (i > 0)
				{
					total += roughHeight[i - 1][j];
					++samples;
				}

				if (i + 1 < 10)
				{
					total += roughHeight[i + 1][j];
					++samples;
				}

				if (j > 0)
				{
					total += roughHeight[i][j - 1];
					++samples;
				}

				if (j + 1 < 10)
				{
					total += roughHeight[i][j + 1];
					++samples;
				}

				smoothHeight[i][j] = total / samples;
			}
		}

		memcpy(roughHeight, smoothHeight, 10 * 10 * sizeof(double));
	}


	for (int i = 0; i < 10; ++i)
	{
		for (int j = 0; j < 10; ++j)
		{
			// Stone floor
			int top = (int)(-10 + smoothHeight[i][j]);
			highestPoint[i][j] = top;
			for (int y = -10; y < top; ++y)
			{
				cubes.push_back(Cube(glm::vec3(i - 5, y, j - 5), STONE));
			}

			// Trees
			if (rand() % 15 == 0 && !(i == 5 && j == 5))
			{
				int treeHeight = 4 + (rand() % 3);
				for (int k = 0; k < treeHeight; ++k)
				{
					cubes.push_back(Cube(glm::vec3(i - 5, top + k, j - 5), TREE));
				}
			}
		}
	}

	// Start up in the air
	glm::vec3(0.5f, 11.5, 0.5f);

	// Load the texture
	glGenTextures(BLOCK_TYPES, textures);
	makeTexture("tree.png", TREE);
	makeTexture("stone.png", STONE);
}

void buildMatrices(const glm::vec3& location)
{
	glm::mat4 projection = glm::perspective(
		45.0f,			// Field of view
		4.0f / 3.0f, 	// Aspect ratio
		0.1f,			// Near clipping plane
		100.0f			// Far clipping plane
	);

	glm::mat4 rotation = glm::rotate(glm::mat4(1.0), camera.horizontalAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	rotation = glm::rotate(rotation, camera.verticalAngle, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::vec3 gaze = glm::mat3(rotation) * glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 up = glm::mat3(rotation) * glm::vec3(0.0, 1.0f, 0.0f);

	glm::mat4 view = glm::lookAt(camera.eye, camera.eye + gaze, up);

	glm::mat4 model = glm::translate(glm::mat4(1.0f), location);
	glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, &model[0][0]);

	glm::mat4 mvp = projection * view * model;
	glUniformMatrix4fv(
		mvpMatrix,	// Id of this uniform variable
		1,			// Number of matrices
		GL_FALSE,	// Transpose
		&mvp[0][0]	// The location of the data
	);
}

void render()
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


	// Fill the screen with blue
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);

	for (auto i = cubes.begin(); i != cubes.end(); ++i)
	{
		glBindTexture(GL_TEXTURE_2D, textures[i->cubeType]);
		buildMatrices(i->location);

		// Draw a cube
		glDrawArrays(GL_TRIANGLES, 0, 3 * 12);
	}
	
	glDisableVertexAttribArray(position);
	glDisableVertexAttribArray(vertexUv);
	glDisableVertexAttribArray(normal);
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

	// Ensure we can capture the escape key being pressed below
	glfwEnable(GLFW_STICKY_KEYS);
	glfwEnable(GLFW_STICKY_MOUSE_BUTTONS);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

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

	double lastTime = glfwGetTime();
	double lastFrame;
 	int nbFrames = 0;

	// Loop until the escape key is pressed or the window is closed
	while (glfwGetWindowParam(GLFW_OPENED))
	{
		// Measure speed
	     double currentTime = glfwGetTime();
	     if (currentTime - lastTime >= 1.0)
	     {
	         std::cout << double(nbFrames) / (currentTime - lastTime) << " fps" << std::endl;
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

		// Speed = 2 blocks / sec
		double blocksPerFrame = 2.0 * lastFrame;

		if (glfwGetKey('W') == GLFW_PRESS)
			camera.eye.z += blocksPerFrame;

		if (glfwGetKey('S') == GLFW_PRESS)
			camera.eye.z -= blocksPerFrame;

		if (glfwGetKey('A') == GLFW_PRESS)
			camera.eye.x += blocksPerFrame;

		if (glfwGetKey('D') == GLFW_PRESS)
			camera.eye.x -= blocksPerFrame;

		// Fall down to ground level
		int i = camera.eye.x + 5;
		int j = camera.eye.z + 5;
		int terrainHeight;
		if (i >= 0 && i < 10 && j >= 0 && j < 10) terrainHeight = highestPoint[i][j];
		else terrainHeight = -1000;

		double heightAboveGround = camera.eye.y - (terrainHeight + 1.5);
		if (heightAboveGround > 0)
			camera.eye.y = std::max(camera.eye.y - blocksPerFrame, terrainHeight + 1.5);
		else if (heightAboveGround < 0)
			camera.eye.y = std::min(camera.eye.y + blocksPerFrame, terrainHeight + 1.5);

		int x, y;
		glfwGetMousePos(&x, &y);

		if (mouseCaptured)
		{
			camera.horizontalAngle -= (x - lastx);
			camera.verticalAngle -= (y - lasty);

			if (camera.verticalAngle < -90.0) camera.verticalAngle = -90.0;
			if (camera.verticalAngle > 90.0) camera.verticalAngle = 90.0;
		}

		lastx = x;
		lasty = y;

		render();

		// Display on the screen
		glfwSwapBuffers();
		nbFrames++;
		lastFrame = glfwGetTime() - currentTime;
	}

	// Close OpenGL window and terminate glfw
	glfwTerminate();
	return 0;
}

