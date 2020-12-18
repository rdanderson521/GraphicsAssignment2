#ifndef LIGHTS_UNIFORM_BLOCK_WRAPPER_H
#define LIGHTS_UNIFORM_BLOCK_WRAPPER_H

#include "wrapper_glfw.h"
#include <vector>
#include <glm/glm.hpp>

enum LightMode {
	DIRECTIONAL,
	OMNI_DIRECTIONAL
};

class LightsUniformWrapper 
{
public:


	static const unsigned int MAX_LIGHTS = 20; // the size of the arrays which hold the lights in the shader uniform block

	bool addPointLight(glm::vec3 pos, bool mode, glm::vec3 colour = glm::vec3(1.f), glm::vec3 attenuation = glm::vec3(1.f,0.f,0.f)); // for point lights
	bool addDirectionalLight(glm::vec3 pos, bool mode, glm::mat4 lightSpace, glm::vec3 colour = glm::vec3(1.f), glm::vec3 attenuation = glm::vec3(1.f,0.f,0.f)); // for directional lights which require the light space matrix to be sent to the shaders

	void resetLights();

	void bind(GLuint bindingPoint);

	// using vec4 insteam of vec3 as layout std140 requires padding in arays to get each object up to 16bytes
	glm::vec4 lightPos[MAX_LIGHTS];
	glm::uint lightMode[MAX_LIGHTS];
	glm::mat4 lightSpace[MAX_LIGHTS];
	glm::vec4 lightColour[MAX_LIGHTS];
	glm::vec4 attenuationParams[MAX_LIGHTS];
	glm::uint numLights;

private:
	// this is padding to be added after each element of lightMode to 
	const glm::vec3 padding = glm::vec3(0.f);

	GLuint uniformBuffer;

	bool bufferChanged;

	void genBuffer();
};

#endif