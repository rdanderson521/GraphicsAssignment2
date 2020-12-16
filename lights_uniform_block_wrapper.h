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
	// this is padding to be added after each element of lightMode to 
	const glm::vec3 padding = glm::vec3(0.f);

	// using vec4 insteam of vec3 as layout std140 requires padding in arays to get each object up to 16bytes
	glm::vec4 lightPos[MAX_LIGHTS];
	glm::uint lightMode[MAX_LIGHTS];
	glm::vec4 lightColour[MAX_LIGHTS];
	glm::vec4 attenuationParams[MAX_LIGHTS];
	glm::uint numLights;

	GLuint uniformBuffer;

	bool bufferChanged;

	void genBuffer();
};

#endif