#ifndef CYLINDER_H
#define CYLINDER_H

#include "wrapper_glfw.h"
#include <vector>
#include <glm/glm.hpp>

class CylinderAttribArrays
{
public:

	CylinderAttribArrays()
	{
		vertices = nullptr;
		normals = nullptr;
		textures = nullptr;
		indecies = nullptr;
		numVertices = 0;
		numIndices = 0;
		topStart = 0;
		topSize = 0;
		bottomStart = 0;
		bottomSize = 0;
		outsideStart = 0;
		outsideSize = 0;
	}

	~CylinderAttribArrays()
	{
		if (vertices != nullptr)
			delete vertices;
		if (normals != nullptr)
			delete normals;
		if (textures != nullptr)
			delete textures;
		if (indecies != nullptr)
			delete indecies;
	}


	GLfloat* vertices;
	GLfloat* normals;
	GLfloat* textures;
	GLuint* indecies;
	int numVertices;
	int numIndices;
	int topStart, topSize, bottomStart, bottomSize, outsideStart, outsideSize;
};

class Cylinder
{
public:
	Cylinder();
	~Cylinder();

	void makeCylinder(GLuint numSegments, float topSize = 1.f);
	CylinderAttribArrays* getCylinderAttribs(GLuint numSegments, float topSize = 1.f);
	void drawCylinder(int drawmode);

	// Define vertex buffer object names (e.g as globals)
	GLuint cylinderBufferObject;
	GLuint cylinderNormals;
	GLuint cylinderColours;
	GLuint cylinderTextures;
	GLuint elementbuffer;

	GLuint attribute_v_coord;
	GLuint attribute_v_normal;
	GLuint attribute_v_colours;

	int numCylinderVertices;
	int numSegments;
	float topSize;

private:
	void makeUnitCylinder(GLfloat* pVertices, GLfloat* pTextures);
};


#endif