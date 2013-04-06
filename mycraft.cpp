#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <GL/glew.h>
#include <GL/glfw.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

// The array of vertices
// x-coordinate is right, y-coordinate is up, and z-coordinate is out of the screen
// The screen ranges from -1 to 1 in the x- and y-coordinates
const GLfloat vertices[] =
{
	1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  0.0f,
    1.0f,  0.0f,  1.0f,
    1.0f,  0.0f,  0.0f,
    0.0f,  1.0f,  1.0f,
    0.0f,  1.0f,  0.0f,
    0.0f,  0.0f,  1.0f,
    0.0f,  0.0f,  0.0f,
};

// The array of vertex indices making up the elements
const GLubyte elements[][3] =
{
	{1, 0, 2},
	{2, 3, 1},

	{0, 4, 6},
	{6, 2, 0},

	{7, 6, 4},
	{4, 5, 7},

	{1, 5, 7},
	{7, 3, 1},

	{7, 3, 2},
	{2, 6, 7},

	{1, 5, 4},
	{4, 0, 1},
};


GLuint vertexBuffer, elementBuffer;
GLint position, mvpMatrix, cubeType;
GLuint programId;

struct Cube
{
	Cube(const glm::vec3& location, GLuint cubeType)
	: location(location), cubeType(cubeType)
	{}

	glm::vec3 location;
	GLuint cubeType;
};

std::vector<Cube> cubes;

void initialize()
{
	// Create a vertex array object
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create a vertex buffer and send the vertex data
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Create an element buffer and send the element data
	glGenBuffers(1, &elementBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

	// Load, compile, and link the shaders
	GLuint vertexShader = loadShader("vertex.glsl", GL_VERTEX_SHADER);
	GLuint fragmentShader = loadShader("fragment.glsl", GL_FRAGMENT_SHADER);
	programId = linkShaders(vertexShader, fragmentShader);

	// Get the attribute id of the input variable "position" to the vertex shader
	position = glGetAttribLocation(programId, "position");

	// Get ids for the uniform variables
	mvpMatrix = glGetUniformLocation(programId, "mvpMatrix");
	cubeType = glGetUniformLocation(programId, "cubeType");

	for (int x = -5; x <= 5; ++x)
	{
		for (int y = -5; y <= 5; ++y)
		{
			for (int z = -5; z <= 5; ++z)
			{
				if (rand() % 10 == 0)
				{
					cubes.push_back(Cube(glm::vec3(x, y, z), rand() % 2));
				}
			}
		}
	}
}

glm::mat4 buildMatrix(const glm::vec3& location)
{
	glm::mat4 projection = glm::perspective(
		45.0f,		// Field of view
		4.0f / 3.0f, // Aspect ratio
		0.1f,		// Near clipping plane
		100.0f		// Far clipping plane
	);

	glm::mat4 view = glm::lookAt(
		glm::vec3(4, 3, 3),	// Camera location in world coordinates
		glm::vec3(0, 0, 0),	// Looking at the origin
		glm::vec3(0, 1, 0)	// Camera up vector
	);

	glm::mat4 model = glm::translate(glm::mat4(1.0f), location);

	return projection * view * model;
}

void render()
{
	glUseProgram(programId);

	// Bind vertexBuffer to GL_ARRAY_BUFFER
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	glVertexAttribPointer(
		position,           // This is the id we got for the "position" input variable
	   	3,                  // number of components
	   	GL_FLOAT,           // type
	   	GL_FALSE,           // normalize?
	   	0,                  // stride (no empty space in array)
	   	nullptr    		    // array buffer offset
	);

	glEnableVertexAttribArray(position);

	// Fill the screen with blue
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);

	for (auto i = cubes.begin(); i != cubes.end(); ++i)
	{
		glUniform1ui(cubeType, i->cubeType);

		glm::mat4 mvp = buildMatrix(i->location);
		glUniformMatrix4fv(
			mvpMatrix,	// Id of this uniform variable
			1,			// Number of matrices
			GL_FALSE,	// Transpose
			&mvp[0][0]	// The location of the data
		);
		 
		// Draw a cube
		glDrawElements(
		    GL_TRIANGLES,  		// mode
		    3 * 12,             // count
		    GL_UNSIGNED_BYTE,   // type
		    nullptr             // element array buffer offset
		);
	}
	
	glDisableVertexAttribArray(position);
}

int main()
{
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

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	initialize();

	// Loop until the escape key is pressed or the window is closed
	while (glfwGetKey(GLFW_KEY_ESC) != GLFW_PRESS &&
		   glfwGetWindowParam(GLFW_OPENED))
	{
		render();

		// Display on the screen
		glfwSwapBuffers();
	}

	// Close OpenGL window and terminate glfw
	glfwTerminate();
	return 0;
}

