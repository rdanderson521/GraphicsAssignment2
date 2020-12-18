// Vertex shader demonstrating a positional light
// source with attenuation
// Iain Martin 2018

// Specify minimum OpenGL version
#version 420 core

#define MAX_LIGHTS 20

// Define the vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 colour;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 texCoord;

// This is the output vertex colour sent to the rasterizer
//out vec4 fcolour;

out VERTEX_OUT
{
	vec3 pos;
	vec3 normal;
	vec4 vertexColour;
	vec4 fragLightSpace[MAX_LIGHTS];
	vec2 texCoord;
} vOut;


layout (std140) uniform lightParams	{
	vec3 lightPos[MAX_LIGHTS];
	uint lightMode[MAX_LIGHTS];
	mat4 lightSpace[MAX_LIGHTS];
	vec3 lightColour[MAX_LIGHTS];
	vec3 attenuationParams[MAX_LIGHTS];
	uint numLights;
} lights;


uniform mat4 model, view, projection;

void main()
{
	vOut.vertexColour = vec4(colour,1.0);
	vOut.pos = vec3(model * vec4(position, 1.f));
	vOut.normal = normal; 
	for (int i = 0; i < lights.numLights; i++)
	{
		vOut.fragLightSpace[i] = lights.lightSpace[i] * vec4(vOut.pos,1.f);
	}
	vOut.texCoord = vec2(position.x,position.z);

	gl_Position = (projection * view * model) * vec4(position, 1.0);
}


