// FirstOpenGL.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "iostream"
#include "GL/glew.h"
#include "GLFW/glfw3.h"

#pragma comment ( lib, "opengl32.lib" )
#pragma comment ( lib, "lib/glew32.lib" )
#pragma comment ( lib, "lib/glfw3.lib" )

int main(void)
{
	GLFWwindow* window;

	// Initialize the library 
	if (!glfwInit())
		return -1;

	// Create a windowed mode window and its OpenGL context 
	window = glfwCreateWindow(640, 480, "Hello DSA", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	// Make the window's context current 
	glfwMakeContextCurrent(window);

	// output info about opengl
	const char* vendor = (const char*)glGetString(GL_VENDOR);
	const char* renderer = (const char*)glGetString(GL_RENDERER);
	const char* version = (const char*)glGetString(GL_VERSION);
	const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
	std::cout << vendor << std::endl;
	std::cout << renderer << std::endl;
	std::cout << version << std::endl;
	std::cout << extensions << std::endl;

	// Loop until the user closes the window 
	while (!glfwWindowShouldClose(window))
	{
		// Render here - this calls an opengl function
		glClear(GL_COLOR_BUFFER_BIT);

		// Swap front and back buffers 
		glfwSwapBuffers(window);

		// Poll for and process events 
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

