#include "directional_light.h"

#include <vector>

#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

glm::mat4 DirectionalLight::genLightProjView(glm::mat4 view, glm::mat4 proj)
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

	this->lightSpace = shadowProjection * shadowView;

	return this->lightSpace;
}


bool DirectionalLight::setUniforms(LightsUniformWrapper& uniform)
{
	return uniform.addDirectionalLight(this->dir, LightMode::DIRECTIONAL, this->lightSpace,vec3(1.f), vec3(0.8f,0.f,0.f));
}