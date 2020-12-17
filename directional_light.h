#ifndef DIRECTIONAL_LIGHT_H
#define DIRECTIONAL_LIGHT_H

#include "wrapper_glfw.h"
#include <vector>
#include <glm/glm.hpp>

#include "lights_uniform_block_wrapper.h"

class DirectionalLight
{
public:
	glm::vec3 dir;
	glm::mat4 lightSpace;

	// this method generates the projection*view matrix for the light given the 
	// projection and view matrices of the camera
	glm::mat4 genLightProjView(glm::mat4 view, glm::mat4 proj);

	bool setUniforms(LightsUniformWrapper& uniform);

};


#endif