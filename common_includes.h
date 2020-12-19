#pragma once

#define MAX_LIGHTS 20

enum programID
{
	MAIN_PROGRAM,
	TERRAIN_PROGRAM,
	SHADOW_PROGRAM,
	OMNI_SHADOW_PROGRAM,
	NUM_PROGRAMS
};

enum UniformBinding
{
	LIGHT_PARAMS_BINDING = 0
};

enum textures
{
	SAND,
	GRASS,
	DIRT,
	ROCK,
	BARK,
	CARBON,
	totalNumTextures
};


// the uniforms needed by the drone class for rendering
struct DroneUniforms
{
	GLuint *textureID,
		*roughnessID,
		*useTextureID,
		*useRoughnessID,
		*modelID,
		*normalMatrixID,
		*colourOverrideID,
		*reflectivenessID;
};