#include "lights_uniform_block_wrapper.h"
#include "common_includes.h"

#include <iostream>

using namespace glm;

bool LightsUniformWrapper::addPointLight(glm::vec3 pos, bool mode, glm::vec3 colour, glm::vec3 attenuation)
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
		this->shadowIdx[this->numLights] = 0;
		this->cascading[this->numLights] = false;
		this->numLights++;
		//genBuffer();
	}
	else
	{
		return false;
	}
}

bool LightsUniformWrapper::addDirectionalLight(glm::vec3 pos, bool mode, std::vector<glm::mat4>& lightSpace, bool cascading, glm::vec3 colour, glm::vec3 attenuation)
{
	// this loop checks if the light source being addedis a duplicate and if it is returns true as it is already part of the structure
	for (int i = 0; i < this->numLights; i++)
	{
		if (this->lightPos[i] == vec4(pos, 0.f) &&
			this->lightMode[i] == mode &&
			//this->lightSpace[i] == lightSpace &&
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
		for (int i = 0; i < lightSpace.size() && i < NUM_FAR_PLANES; i++)
		{
			this->lightSpace[(this->numLights * NUM_FAR_PLANES) + i] = lightSpace.at(i);
		}
		for (int i = lightSpace.size(); i < NUM_FAR_PLANES; i++)
		{
			this->lightSpace[(this->numLights * NUM_FAR_PLANES) + i] = mat4(1.f);
		}
		this->lightColour[this->numLights] = vec4(colour, 0.f);
		this->attenuationParams[this->numLights] = vec4(attenuation, 0.f);
		this->shadowIdx[this->numLights] = 0;
		this->cascading[this->numLights] = cascading;
		this->numLights++;
		genBuffer();
	}
	else
	{
		return false;
	}
}

void LightsUniformWrapper::resetLights()
{
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		this->lightPos[i] = glm::vec4(0.f);
		this->lightMode[i] = 0;
		for (int j = 0; j < NUM_FAR_PLANES; j++)
			this->lightSpace[i * MAX_LIGHTS + j] = mat4(1.f);
		this->lightColour[i] = vec4(vec3(1.f),0.f);
		this->attenuationParams[i] = vec4(1.f, 0.f, 0.f, 0.f);
		this->shadowIdx[i] = 0;
		this->cascading[i] = false;
		this->numLights = 0;
	}
}

void LightsUniformWrapper::bind(GLuint bindingPoint)
{
	this->genBuffer();
	glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, uniformBuffer);
}

void LightsUniformWrapper::genBuffer()
{
	glGenBuffers(1, &uniformBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);

	size_t offset = 0;

	glBufferData(GL_UNIFORM_BUFFER, ((6 * sizeof(vec4)) + (NUM_FAR_PLANES * sizeof(mat4))) * MAX_LIGHTS + sizeof(GLuint), NULL, GL_STATIC_DRAW);

	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(vec4) * MAX_LIGHTS, lightPos);
	offset += sizeof(vec4) * MAX_LIGHTS;
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(uint) , &lightMode[i]);
		offset += sizeof(uint);
		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(vec3) , &padding);
		offset += sizeof(vec3);
	}

	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(mat4) * NUM_FAR_PLANES * MAX_LIGHTS, lightSpace);
	offset += sizeof(mat4) * NUM_FAR_PLANES * MAX_LIGHTS;

	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(vec4) * MAX_LIGHTS, lightColour);
	offset += sizeof(vec4) * MAX_LIGHTS;
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(vec4) * MAX_LIGHTS, attenuationParams);
	offset += sizeof(vec4) * MAX_LIGHTS;

	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(uint), &shadowIdx[i]);
		offset += sizeof(uint);
		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(vec3) , &padding);
		offset += sizeof(vec3);
	}

	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(uint), &cascading[i]);
		offset += sizeof(uint);
		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(vec3), &padding);
		offset += sizeof(vec3);
	}

	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(uint), &numLights);

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}