#ifndef DRONE_H
#define DRONE_H

/* Include GLM core and matrix extensions*/
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

#include "cubev2.h"
#include "Tube.h"
#include "cylinder.h"

#include "lights_uniform_block_wrapper.h"

class Drone
{
public:
	Cubev2 cube;
	Tube motorBell;
	Cylinder motorStator, cylinder;

	float x, y, z; //position


	void init();

	void draw(int drawmode);

	void setLightUniforms(LightsUniformWrapper& uniforms);

	void move();

private:

	vec3 framePlateScale = vec3(1.f, 0.015f, 0.3f);
	vec3 frameArmScale = vec3(0.8f, 0.03f, 0.15f);
	vec3 standoffScale = vec3(0.025f, 0.17f, 0.025f);

	vec3 motorBellScale = vec3(0.15f, 0.085f, 0.15f);
	vec3 motorStatorScale = vec3(0.125f, 0.08f, 0.125f);
	vec3 motorShaftScale = vec3(0.025f, 0.085f, 0.025f);
	vec3 motorStrutsScale = vec3(0.011f, 0.011f, 0.14f);
	vec3 groundPlaneScale = vec3(20.f, 0.0001f, 20.f);

	vec4 frameColour = vec4(0.20f, 0.20f, 0.20f, 1.f);
	vec4 treeColour = vec4(0.50f, 0.25f, 0.f, 1.f);
	vec4 motorColour = vec4(0.60f, 0.60f, 0.60f, 1.f);
	vec4 motorStatorColour = vec4(0.88f, 0.44f, 0.f, 1.f);
	vec4 standoffColour = vec4(1.f, 0.f, 0.f, 1.f);
	vec3 groundPlaneColour = vec3(0.8f, 0.8f, 0.8f);

	GLfloat motorReflect = 8.f;
	GLfloat motorStatorReflect = 2.f;
	GLfloat standoffReflect = 1.f;
};

#endif