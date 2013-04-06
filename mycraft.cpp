#include <iostream>
#include <GL/glew.h>
#include <GL/glfw.h>
#include <glm/glm.hpp>

int main()
{
	// Initialize glfw
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize glfw" << std::endl;
		return 1;
	}

	// Use glfw to open a window
	glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);
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

	// Loop until the escape key is pressed or the window is closed
	while (glfwGetKey(GLFW_KEY_ESC) != GLFW_PRESS &&
		   glfwGetWindowParam(GLFW_OPENED))
	{
		// Render here

		glfwSwapBuffers();
	}

	// Close OpenGL window and terminate glfw
	glfwTerminate();
	return 0;
}

