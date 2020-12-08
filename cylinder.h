#ifndef CYLINDER_H
#define CYLINDER_H

#include "wrapper_glfw.h"
#include <vector>
#include <glm/glm.hpp>

struct CylinderAttribArrays
{
	GLfloat vertices;
	GLfloat normals;
	GLfloat indecies;
	int numVertices;
	int numIndicies;
};

class Cylinder
{
public:
	Cylinder();
	~Cylinder();

	void makeCylinder(GLuint numSegments);
	static CylinderAttribArrays* getCylinderAttribs(GLuint numSegments);
	void drawCylinder(int drawmode);

	// Define vertex buffer object names (e.g as globals)
	GLuint cylinderBufferObject;
	GLuint cylinderNormals;
	GLuint cylinderColours;
	GLuint elementbuffer;

	GLuint attribute_v_coord;
	GLuint attribute_v_normal;
	GLuint attribute_v_colours;

	int numCylinderVertices;
	int numSegments;

private:
	void makeUnitCylinder(GLfloat* pVertices);
};


#endif