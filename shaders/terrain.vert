// Vertex shader demonstrating a positional light
// source with attenuation
// Iain Martin 2018

// Specify minimum OpenGL version
#version 420 core

#define MAX_LIGHTS 5
#define NUM_FAR_PLANES 3

// Define the vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 colour;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 texCoord;

out VERTEX_OUT
{
	vec3 pos;
	vec3 normal;
	vec4 vertexColour;
	vec4 fragLightSpace[MAX_LIGHTS * NUM_FAR_PLANES];
	vec2 texCoord;
	mat4 modelView;
} vOut;

flat out uint cascadingIdx;

layout (std140) uniform lightParams	{
	vec3 lightPos[MAX_LIGHTS];
	uint lightMode[MAX_LIGHTS];
	mat4 lightSpace[MAX_LIGHTS * NUM_FAR_PLANES];
	vec3 lightColour[MAX_LIGHTS];
	vec3 attenuationParams[MAX_LIGHTS];
	uint shadowIdx[MAX_LIGHTS];
	bool cascading[MAX_LIGHTS];
	uint numLights;
} lights;


uniform mat4 model, view, projection;

uniform float farPlanes[NUM_FAR_PLANES];

void main()
{
	vOut.vertexColour = vec4(colour,1.0);
	vOut.pos = vec3(model * vec4(position, 1.f));
	vOut.normal = normal; 
	for (int i = 0; i < MAX_LIGHTS * NUM_FAR_PLANES; i++)
	{
		vOut.fragLightSpace[i] = lights.lightSpace[i] * vec4(vOut.pos,1.f);
	}
	vOut.texCoord = vec2(position.x,position.z);

	vOut.modelView = view * model;

	float zpos = ((projection * view * model) * vec4(position, 1.0)).z;// * farPlanes[NUM_FAR_PLANES - 1];
	if (abs(zpos) < farPlanes[0])
	{
		cascadingIdx = 0;
	}
	else if (abs(zpos) < farPlanes[1])
	{
		cascadingIdx = 1;
	}
	else 
	{
		cascadingIdx = 2;
	}
	

	gl_Position = (projection * view * model) * vec4(position, 1.0);
}


