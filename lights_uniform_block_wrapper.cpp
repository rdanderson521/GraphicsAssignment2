#include "lights_uniform_block_wrapper.h"

using namespace glm;

bool LightsUniformWrapper::addLight(glm::vec3 pos, bool mode, glm::vec3 colour, glm::vec3 attenuation)
{
	// this loop checks if the light source being addedis a duplicate and if it is returns true as it is already part of the structure
	for (int i = 0; i < this->numLights; i++)
	{
		if (this->lightPos[i] == pos &&
			this->lightMode[i] == mode &&
			this->lightColour[i] == colour &&
			this->attenuationParams[i] == attenuation) 
		{
			return true;
		}
	}

	if (this->numLights < MAX_LIGHTS)
	{
		this->lightPos[this->numLights] = pos;
		this->lightMode[this->numLights] = mode;
		this->lightColour[this->numLights] = colour;
		this->attenuationParams[this->numLights] = attenuation;
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
		this->lightPos[i] = glm::vec3(0.f);
		this->lightMode[i] = 0;
		this->lightColour[i] = glm::vec3(1.f);
		this->attenuationParams[i] = glm::vec3(1.f, 0.f, 0.f);
		this->numLights = 0;
	}
}

void LightsUniformWrapper::bind(GLuint bindingPoint)
{
	//if (this->bufferChanged)
	//{
		this->genBuffer();
	//}
	glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, uniformBuffer);
}

void LightsUniformWrapper::genBuffer()
{
	glGenBuffers(1, &uniformBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);

	glBufferData(GL_UNIFORM_BUFFER, (3 * sizeof(vec3) + sizeof(uint)) * MAX_LIGHTS + sizeof(GLuint), NULL, GL_STATIC_DRAW);

	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(vec3) * MAX_LIGHTS, &(lightPos[0]));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(vec3) * MAX_LIGHTS, sizeof(uint) * MAX_LIGHTS, &(lightMode[0]));
	glBufferSubData(GL_UNIFORM_BUFFER, (sizeof(vec3) + sizeof(uint)) * MAX_LIGHTS, sizeof(vec3) * MAX_LIGHTS, &(lightColour[0]));
	glBufferSubData(GL_UNIFORM_BUFFER, (2 * sizeof(vec3) + sizeof(uint)) * MAX_LIGHTS, sizeof(vec3) * MAX_LIGHTS, &(attenuationParams[0]));
	glBufferSubData(GL_UNIFORM_BUFFER, (3 * sizeof(vec3) + sizeof(uint)) * MAX_LIGHTS, sizeof(uint), &numLights);

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}