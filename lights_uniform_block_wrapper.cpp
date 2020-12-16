#include "lights_uniform_block_wrapper.h"

using namespace glm;

bool LightsUniformWrapper::addLight(glm::vec3 pos, bool mode, glm::vec3 colour, glm::vec3 attenuation)
{
	// this loop checks if the light source being addedis a duplicate and if it is returns true as it is already part of the structure
	for (int i = 0; i < this->numLights; i++)
	{
		if (this->lightPos[i] == vec4(pos,0.f) &&
			this->lightMode[i] == mode &&
			this->lightColour[i] == vec4(colour, 0.f) &&
			this->attenuationParams[i] == vec4(attenuation, 0.f))
		{
			return true;
		}
	}

	if (this->numLights < MAX_LIGHTS)
	{
		this->lightPos[this->numLights] = vec4(pos, 0.f);
		this->lightMode[this->numLights] = mode;
		this->lightColour[this->numLights] = vec4(colour, 0.f);
		this->attenuationParams[this->numLights] = vec4(attenuation, 0.f);
		this->numLights++;
		this->bufferChanged = true;
	}
	else
	{
		return false;
	}
}

void LightsUniformWrapper::resetLights()
{
	for (int i = 0; i < this->MAX_LIGHTS; i++)
	{
		this->lightPos[i] = glm::vec4(0.f);
		this->lightMode[i] = 0;
		this->lightColour[i] = vec4(vec3(1.f),0.f);
		this->attenuationParams[i] = vec4(1.f, 0.f, 0.f, 0.f);
		this->numLights = 0;
	}
}

void LightsUniformWrapper::bind(GLuint bindingPoint)
{
	if (this->bufferChanged)
	{
		this->genBuffer();
	}
	glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, uniformBuffer);
}

void LightsUniformWrapper::genBuffer()
{
	glGenBuffers(1, &uniformBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);

	glBufferData(GL_UNIFORM_BUFFER, 4 * sizeof(vec4) * MAX_LIGHTS + sizeof(GLuint), NULL, GL_STATIC_DRAW);

	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(vec4) * MAX_LIGHTS, lightPos);

	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(vec4) * (MAX_LIGHTS + i), sizeof(uint) * MAX_LIGHTS, &lightMode[i]);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(vec4) * (MAX_LIGHTS + i) + sizeof(uint), sizeof(vec3) * MAX_LIGHTS, &padding);
	}

	glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(vec4) * MAX_LIGHTS, sizeof(vec4) * MAX_LIGHTS, lightColour);
	glBufferSubData(GL_UNIFORM_BUFFER, 3 * sizeof(vec4) * MAX_LIGHTS, sizeof(vec4) * MAX_LIGHTS, attenuationParams);
	glBufferSubData(GL_UNIFORM_BUFFER, 4 * sizeof(vec4) * MAX_LIGHTS, sizeof(uint), &numLights);

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}