// Vertex shader demonstrating a positional light
// source with attenuation
// Iain Martin 2018

// Specify minimum OpenGL version
#version 420 core

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
	vec4 FragPosLightSpace;
	vec2 texCoord;
} vOut;



// These are the uniforms that are defined in the application
uniform mat4 model, view, projection;
uniform mat4 lightSpaceMatrix;

void main()
{
	vOut.vertexColour = vec4(colour,1.0);
	vOut.pos = vec3(model * vec4(position, 1.f));
	vOut.normal = normal; 
	vOut.FragPosLightSpace = lightSpaceMatrix * vec4(vOut.pos,1.f);
	vOut.texCoord = vec2(position.x,position.z);

	gl_Position = (projection * view * model) * vec4(position, 1.0);
}


