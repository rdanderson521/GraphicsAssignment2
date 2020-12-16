#ifndef LIGHTS_UNIFORM_BLOCK_WRAPPER_H
#define LIGHTS_UNIFORM_BLOCK_WRAPPER_H

#include "wrapper_glfw.h"
#include <vector>
#include <glm/glm.hpp>

class LightsUniformWrapper 
{
public:


	static const unsigned int MAX_LIGHTS = 20; // the size of the arrays which hold the lights in the shader uniform block

	bool addLight(glm::vec3 pos, bool mode, glm::vec3 colour = glm::vec3(1.f), glm::vec3 attenuation = glm::vec3(1.f,0.f,0.f));

	void resetLights();

	void bind(GLuint bindingPoint);

private:
	glm::vec3 lightPos[MAX_LIGHTS];
	glm::uint lightMode[MAX_LIGHTS];
	glm::vec3 lightColour[MAX_LIGHTS];
	glm::vec3 attenuationParams[MAX_LIGHTS];
	glm::uint numLights;

	GLuint uniformBuffer;

	bool bufferChanged;

	void genBuffer();
};

#endif