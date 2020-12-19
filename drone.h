#ifndef DRONE_H
#define DRONE_H

/* Include GLM core and matrix extensions*/
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "cubev2.h"
#include "Tube.h"
#include "cylinder.h"

#include "lights_uniform_block_wrapper.h"

#include "common_includes.h"

#include <stack>

class Drone
{
public:
	const int MOTOR_RATE = 47; // degrees which the propellersspin per frame

	// individual objects which make up the drone
	Cubev2 cube;
	Tube motorBell;
	Cylinder motorStator, cylinder;

	// position info
	glm::vec3 pos, orient;

	// scale
	float droneScale;

	// the current rotation of the motor
	float motorAngle;

	// textures
	GLuint frameTexture;
	GLuint frameRoughnessMap;
	
	DroneUniforms u;

	void init(DroneUniforms uniforms, GLuint tex, GLuint rough, glm::vec3 pos = glm::vec3(0.f), glm::vec3 orient = glm::vec3(0.f));

	void draw(int drawmode, GLuint programID, glm::mat4 view);

	void setLightUniforms(LightsUniformWrapper& uniforms, glm::mat4 view);

	void spinMotor();

private:

	// parameters used to transform the parts which make up the drone model
	glm::vec3 framePlateScale = glm::vec3(1.f, 0.015f, 0.3f);
	glm::vec3 frameArmScale = glm::vec3(0.8f, 0.03f, 0.15f);
	glm::vec3 standoffScale = glm::vec3(0.025f, 0.17f, 0.025f);

	glm::vec3 motorBellScale = glm::vec3(0.15f, 0.085f, 0.15f);
	glm::vec3 motorStatorScale = glm::vec3(0.125f, 0.08f, 0.125f);
	glm::vec3 motorShaftScale = glm::vec3(0.025f, 0.085f, 0.025f);
	glm::vec3 motorStrutsScale = glm::vec3(0.011f, 0.011f, 0.14f);
	glm::vec3 groundPlaneScale = glm::vec3(20.f, 0.0001f, 20.f);

	glm::vec4 frameColour = glm::vec4(0.20f, 0.20f, 0.20f, 1.f);
	glm::vec4 treeColour = glm::vec4(0.50f, 0.25f, 0.f, 1.f);
	glm::vec4 motorColour = glm::vec4(0.60f, 0.60f, 0.60f, 1.f);
	glm::vec4 motorStatorColour = glm::vec4(0.88f, 0.44f, 0.f, 1.f);
	glm::vec4 standoffColour = glm::vec4(1.f, 0.f, 0.f, 1.f);
	glm::vec3 groundPlaneColour = glm::vec3(0.8f, 0.8f, 0.8f);

	GLfloat motorReflect = 125.f;
	GLfloat motorStatorReflect = 2.f;
	GLfloat standoffReflect = 1.f;

	void globalTransformations(std::stack<glm::mat4>& model);
};

#endif