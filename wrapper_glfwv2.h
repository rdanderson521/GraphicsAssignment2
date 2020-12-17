/**
wrapper_glfw.h
Modified from the OpenGL GLFW example to provide a wrapper GLFW class
Iain Martin August 2014

wrapper_glfwv2.h
Modified to also have function to load geometery shader,name changed so it is not mistaken 
with the files provided in the examples
Ryan Anderson 2020

*/
#pragma once

#include <string>

/* Inlcude GL_Load and GLFW */
#include <glload/gl_4_0.h>
#include <glload/gl_load.h>
#include <GLFW/glfw3.h>

class GLWrapperV2 {
private:

	int width;
	int height;
	const char *title;
	double fps;
	void(*renderer)();
	bool running;
	GLFWwindow* window;

public:
	GLWrapperV2(int width, int height, const char *title);
	~GLWrapperV2();

	void setFPS(double fps) {
		this->fps = fps;
	}

	void DisplayVersion();

	/* Callback registering functions */
	void setRenderer(void(*f)());
	void setReshapeCallback(void(*f)(GLFWwindow* window, int w, int h));
	void setKeyCallback(void(*f)(GLFWwindow* window, int key, int scancode, int action, int mods));
	void setErrorCallback(void(*f)(int error, const char* description));

	/* Shader load and build support functions */
	GLuint LoadShader(const char *vertex_path, const char *fragment_path);
	GLuint LoadShader(const char* vertex_path, const char* geometery_path, const char* fragment_path);

	GLuint BuildShader(GLenum eShaderType, const std::string &shaderText);
	GLuint BuildShaderProgram(std::string vertShaderStr, std::string fragShaderStr);
	std::string readFile(const char *filePath);

	int eventLoop();
	GLFWwindow* getWindow();
};



