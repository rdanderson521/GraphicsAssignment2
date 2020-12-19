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

struct DroneUniforms
{
	GLuint textureID[NUM_PROGRAMS],
		roughnessID[NUM_PROGRAMS],
		useTextureID[NUM_PROGRAMS],
		useRoughnessID[NUM_PROGRAMS],
		modelID[NUM_PROGRAMS],
		normalMatrixID[NUM_PROGRAMS],
		colourOverrideID[NUM_PROGRAMS],
		reflectivenessID[NUM_PROGRAMS];
};