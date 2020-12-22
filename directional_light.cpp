#include "directional_light.h"

#include <vector>
#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

using namespace std;
using namespace glm;

glm::mat4 DirectionalLight::genLightProjView(glm::mat4 view, glm::mat4 proj, int lightSpaceIdx)
{
	mat4 shadowView = glm::lookAt(
		vec3(0.f, 0.f, 0.f),
		this->dir,
		vec3(0.0f, 1.0f, 0.0f));

	mat3 lightVectors = shadowView;

	// below code gets the points in the cameras frustum in world 
	vector<vec4> cubeNDC;
	cubeNDC.push_back(vec4(-1.0f, -1.0f, -1.0f, 1.0f));
	cubeNDC.push_back(vec4(1.0f, -1.0f, -1.0f, 1.0f));
	cubeNDC.push_back(vec4(1.0f, -1.0f, 1.0f, 1.0f));
	cubeNDC.push_back(vec4(-1.0f, -1.0f, 1.0f, 1.0f));
	cubeNDC.push_back(vec4(-1.0f, 1.0f, -1.0f, 1.0f));
	cubeNDC.push_back(vec4(1.0f, 1.0f, -1.0f, 1.0f));
	cubeNDC.push_back(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	cubeNDC.push_back(vec4(-1.0f, 1.0f, 1.0f, 1.0f));

	mat4 renderViewProjectionToLightSPace = shadowView * inverse(proj * view);

	vector<vec4> cameraFrustum;
	for (vec4 vertex : cubeNDC) {
		vec4 vertexTransformed = renderViewProjectionToLightSPace * vertex;
		vertexTransformed /= vertexTransformed.w;
		cameraFrustum.push_back(vertexTransformed);
	}

	vec3 minBound, maxBound;
	for (int i = 0; i < 3; i++)
	{
		bool first = true;
		for (vec3 vertex : cameraFrustum)
		{
			if (first)
			{
				minBound[i] = maxBound[i] = vertex[i];
				first = false;
			}
			else
			{
				minBound[i] = std::min(minBound[i], vertex[i]);
				maxBound[i] = std::max(maxBound[i], vertex[i]);
			}
		}
	}

	mat4 shadowProjection = ortho(minBound.x, maxBound.x, minBound.y, maxBound.y, -maxBound.z - 5, -minBound.z);


	this->lightSpace.at(lightSpaceIdx) = shadowProjection * shadowView;

	return this->lightSpace.at(lightSpaceIdx);
}


bool DirectionalLight::setUniforms(LightsUniformWrapper& uniform)
{
	for (int i = 0; i < this->lightSpace.size(); i++)
	{
		for (int j = 0; j < 4; j++)
		{
			for (int k = 0; k < 4; k++)
				std::cout << this->lightSpace.at(i)[j][k] << " ";
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}
		
	return uniform.addDirectionalLight(this->dir, LightMode::DIRECTIONAL, this->lightSpace, this->cascading, vec3(1.f), vec3(1.2f,0.f,0.f));
}