#include "cylinder.h"

#define PI 3.14159265358979f

#include <iostream>

Cylinder::Cylinder()
{
	attribute_v_coord = 0;
	attribute_v_colours = 1;
	attribute_v_normal = 2;
	numCylinderVertices = 0;
}

Cylinder::~Cylinder()
{}

void Cylinder::makeCylinder(GLuint numSegments)
{
	GLuint numvertices = (numSegments * 2) + 2;

	// Store the number of cylinder vertices in an attribute because we need it later when drawing it
	this->numCylinderVertices = numvertices;
	this->numSegments = numSegments;

	// Create the temporary arrays to store vertex info
	GLfloat* pVertices = new GLfloat[numvertices * 3];
	GLfloat* pNormals = new GLfloat[numvertices * 3];
	GLfloat* pColours = new GLfloat[numvertices * 4];
	this->makeUnitCylinder(pVertices);

	for (int i = 0; i < numvertices; i++)
	{
		pColours[i * 4] = pVertices[i * 3];
		pColours[i * 4 + 1] = pVertices[i * 3 + 1];
		pColours[i * 4 + 2] = pVertices[i * 3 + 2];
		pColours[i * 4 + 3] = 1.f;
	}

	// sets normals for top and bottom disks
	for (int i = 0; i < numvertices / 2; i++)
	{
		pNormals[i * 3] = 0;
		pNormals[(i * 3) + 1] = 0;
		pNormals[(i * 3) + 2] = pVertices[(i * 3) + 2];
	}

	// sets normals for curved surface
	for (int i = numvertices / 2; i < numvertices; i++)
	{
		pNormals[i * 3] = pVertices[(i * 3)];
		pNormals[(i * 3) + 1] = pVertices[(i * 3) + 1];
		pNormals[(i * 3) + 2] = 0;
	}


	/* Generate the vertex buffer object */
	glGenBuffers(1, &this->cylinderBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, this->cylinderBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * numvertices * 3, pVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Store the normals in a buffer object */
	glGenBuffers(1, &this->cylinderNormals);
	glBindBuffer(GL_ARRAY_BUFFER, this->cylinderNormals);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * numvertices * 3, pNormals, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Store the colours in a buffer object */
	glGenBuffers(1, &this->cylinderColours);
	glBindBuffer(GL_ARRAY_BUFFER, this->cylinderColours);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * numvertices * 4, pColours, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// 1 repeating vertex for each fan, 2 for the triangle strip around the outside
	GLuint numindices = (numvertices * 2) + 2;
	GLuint* pindices = new GLuint[numindices];

	int currentIndex = 0;
	// create top and bottom fan indices
	for (int i = 0; i < 2; i++)
	{
		// center vertex
		pindices[(this->numSegments + 2) * i] = (this->numSegments + 1) * i;

		for (int j = 0; j <= this->numSegments; j++)
		{
			pindices[((this->numSegments + 2) * i) + j + 1] = ((this->numSegments + 1) * i) + (j % this->numSegments) + 1;
		}
	}

	// create the indices for the outside triangle strip
	for (int i = 0; i <= this->numSegments; i++)
	{
		pindices[((this->numSegments + 2) * 2) + (i * 2)] = (i % this->numSegments) + 1;
		pindices[((this->numSegments + 2) * 2) + (i * 2) + 1] = (this->numSegments + 1) + (i % this->numSegments) + 1;
	}

	for (int i = 0; i < numindices; i++)
	{
		//pindices[i] = i;
		std::cout << "index " << i << ": " << pindices[i] << std::endl;
	}


	// Generate a buffer for the indices
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numindices * sizeof(GLuint), pindices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	delete[] pindices;
	delete[] pColours;
	delete[] pVertices;
}

CylinderAttribArrays* Cylinder::getCylinderAttribs(GLuint numSegments)
{
	CylinderAttribArrays* a = new CylinderAttribArrays;
	return a;
}


void Cylinder::drawCylinder(int drawmode)
{
	GLuint i;

	/* Draw the vertices as GL_POINTS */
	glBindBuffer(GL_ARRAY_BUFFER, this->cylinderBufferObject);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	/* Bind the sphere normals */
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, this->cylinderNormals);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	/* Bind the sphere colours */
	glBindBuffer(GL_ARRAY_BUFFER, this->cylinderColours);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glPointSize(3.f);

	// Enable this line to show model in wireframe
	if (drawmode == 1)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (drawmode == 2)
	{
		glDrawArrays(GL_POINTS, 0, this->numCylinderVertices);
	}
	else
	{
		/* Bind the indexed vertex buffer */
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);


		glDrawElements(GL_TRIANGLE_FAN, this->numSegments + 2, GL_UNSIGNED_INT, (GLvoid*)(0));
		glDrawElements(GL_TRIANGLE_FAN, this->numSegments + 2, GL_UNSIGNED_INT, (GLvoid*)((this->numSegments + 2)*4));

		glDrawElements(GL_TRIANGLE_STRIP, this->numSegments * 2 + 2, GL_UNSIGNED_INT, (GLvoid*)(2*(this->numSegments + 2) * 4));
	}
}


void Cylinder::makeUnitCylinder(GLfloat* pVertices)
{
	float segmentAngleIncrement = (2 * PI) / this->numSegments;

	int numVerticesPerSection = this->numCylinderVertices / 2;


	// centre point
	pVertices[0] = 0.f;
	pVertices[1] = 0.5f;
	pVertices[2] = 0.f;

	// top disk
	for (int i = 1; i < numVerticesPerSection; i++)
	{
		pVertices[i * 3] = 0.5f * sin(segmentAngleIncrement * i);
		pVertices[(i * 3) + 1] = 0.5 ;
		pVertices[(i * 3) + 2] = 0.5f * cos(segmentAngleIncrement * i);

	}

	// centre point
	pVertices[0 + (numVerticesPerSection * 3)] = 0.f;
	pVertices[1 + (numVerticesPerSection * 3)] = -0.5f;
	pVertices[2 + (numVerticesPerSection * 3)] = 0.f;

	// top disk
	for (int i = 1; i < numVerticesPerSection; i++)
	{
		pVertices[(i * 3) + (numVerticesPerSection * 3)] = 0.5f * sin(segmentAngleIncrement * i);
		pVertices[(i * 3) + 1 + (numVerticesPerSection * 3)] = -0.5;
		pVertices[(i * 3) + 2 + (numVerticesPerSection * 3)] = 0.5f * cos(segmentAngleIncrement * i);

	}

	for (int i = 0; i < this->numCylinderVertices; i++)
	{
		std::cout << pVertices[(i * 3) + 0] << " " << pVertices[(i * 3) + 1] << " " << pVertices[(i * 3) + 2] << std::endl;
	}


	// 
	//for (int i = 0; i < numVerticesPerSection * 3; i++)
	//{
	//	// outside vertices
	//	if (i % 3 != 1)
	//	{
	//		// copies the x and y coordinates from the previous disk
	//		pVertices[(numVerticesPerSection * 3) + i] = pVertices[i];
	//	}
	//	else
	//	{
	//		// sets the z coordinate to the negative of the previous disk
	//		pVertices[(numVerticesPerSection * 3) + i] = -pVertices[i];
	//	}
	//}
}