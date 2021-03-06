#include "tree.h"

#include <iostream>
#include <ctime>
#include <vector>

#include "cylinder.h"


Tree::Tree()
{
	this->attribute_v_coord = 0;
	this->attribute_v_colours = 1;
	this->attribute_v_normal = 2;
	this->attribute_v_tex = 3;
}


void Tree::generate(string rule, int levels)
{

	this->nodes.clear();
	this->stripStarts.clear();
	this->stripSizes.clear();
	this->fanStarts.clear();
	this->fanSizes.clear();

	srand(time(NULL));
	stack<mat4> transformations;
	transformations.push(mat4(1.0f));

	this->generateRecurse(rule, levels, transformations);

	CylinderAttribArrays *cA = cylinder.getCylinderAttribs(7,this->BRANCH_SCALE); 

	vector<vec3> verticesTemplate, normalsTemplate;
	for (int i = 0; i < cA->numVertices; i++)
	{
		verticesTemplate.push_back(vec3(cA->vertices[(i * 3)], cA->vertices[(i * 3) + 1], cA->vertices[(i * 3) + 2]));
		normalsTemplate.push_back(vec3(cA->normals[(i * 3)], cA->normals[(i * 3) + 1], cA->normals[(i * 3) + 2]));
	}

	vector<GLuint> indicesTemplate;
	for (int i = 0; i < cA->numIndices; i++)
	{
		indicesTemplate.push_back(cA->indecies[i]);
	}

	unsigned int totalVertices = cA->numVertices * this->nodes.size();
	unsigned int totalIndices = cA->numIndices * this->nodes.size();
	GLfloat* pVertices = new GLfloat[totalVertices * 3];
	GLfloat* pNormals = new GLfloat[totalVertices * 3];
	GLfloat* pColours = new GLfloat[totalVertices * 4];
	GLfloat* pTextures = new GLfloat[totalVertices * 2];
	GLuint* pIndices = new GLuint[totalIndices];

	vec4 treeColour = vec4(0.5f, 0.25f, 0.f, 1.f);

	for (size_t i = 0; i < this->nodes.size(); i++)
	{
		mat4 transform = mat4(1.f);
		transform = transform * this->nodes.at(i).transformation;
		transform = scale(transform, vec3(this->BRANCH_WIDTH, this->BRANCH_HEIGHT, this->BRANCH_WIDTH));
		transform = translate(transform, vec3(0, 0.5, 0));

		for (size_t j = 0; j < cA->numVertices; j++)
		{
			unsigned int currentVertex = (i * cA->numVertices * 3) + (j * 3);
			unsigned int currentColour = (i * cA->numVertices * 4) + (j * 4);
			unsigned int currentTexture = (i * cA->numVertices * 2) + (j * 2);

			vec3 newVertex = transform * vec4(verticesTemplate.at(j), 1);
			pVertices[currentVertex] = newVertex.x;
			pVertices[currentVertex+1] = newVertex.y;
			pVertices[currentVertex+2] = newVertex.z;

			vec3 newNormal = transform * vec4(normalsTemplate.at(j), 1);
			pNormals[currentVertex] = newNormal.x;
			pNormals[currentVertex + 1] = newNormal.y;
			pNormals[currentVertex + 2] = newNormal.z;

			pColours[currentColour] = treeColour.r;
			pColours[currentColour + 1] = treeColour.g;
			pColours[currentColour + 2] = treeColour.b;
			pColours[currentColour + 3] = treeColour.a;

			pTextures[currentTexture] = cA->textures[(j*2)];
			pTextures[currentTexture + 1] = cA->textures[(j * 2) + 1];
		}

		for (size_t j = 0; j < cA->numIndices; j++)
		{
			pIndices[(i * cA->numIndices) + j] = (GLuint)((i * cA->numVertices) + indicesTemplate.at(j));
		}

		this->fanStarts.push_back(cA->topStart + (i * cA->numIndices));
		this->fanSizes.push_back(cA->topSize);
		this->fanStarts.push_back(cA->bottomStart + (i * cA->numIndices));
		this->fanSizes.push_back(cA->bottomSize);

		this->stripStarts.push_back(cA->outsideStart + (i * cA->numIndices));
		this->stripSizes.push_back(cA->outsideSize);
	}

	/* Generate the vertex buffer object */
	glGenBuffers(1, &this->positionBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, this->positionBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * totalVertices * 3, pVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Store the normals in a buffer object */
	if (this->normalsBufferObject == 0)
	glGenBuffers(1, &this->normalsBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, this->normalsBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * totalVertices * 3, pNormals, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Store the colours in a buffer object */
	glGenBuffers(1, &this->colourBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, this->colourBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * totalVertices * 4, pColours, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Store the texture coordinates in a buffer object */
	glGenBuffers(1, &this->textureBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, this->textureBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * totalVertices * 2, pTextures, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Generate a buffer for the indices
	glGenBuffers(1, &this->elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * totalIndices, pIndices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	delete cA;
}

void Tree::generateRecurse(string rule, int levels, stack<mat4>& transformations)
{
	if (transformations.empty())
	{
		transformations.push(mat4(1.0f));
	}

	for (int i = 0; i < rule.size(); i++)
	{
		switch (rule.at(i))
		{
			case ('F'): // draw branch or leaf ( or recurse when not level > 0)
			{
				if (levels == 0)
				{
					// draw branch or leaf
					TreeNode newNode;

					newNode.transformation = transformations.top();

					this->nodes.push_back(newNode);

					transformations.top() = translate(transformations.top(), vec3(0, this->BRANCH_HEIGHT, 0));				
					
				}
				else
				{
					this->generateRecurse(rule, levels - 1, transformations);
				}
				break;
			}
			case ('['): // start new branch
			{
				transformations.push(transformations.top());
				transformations.top() = scale(transformations.top(), vec3(BRANCH_SCALE));
				break;
			}
			case (']'): // close current branch
			{
				transformations.pop();
				break;
			}
			case ('+'): // positive rotation
			{
				transformations.top() = rotate(transformations.top(), radians((GLfloat)(rand() % 120)) , glm::vec3(0, 1, 0));
				transformations.top() = rotate(transformations.top(), radians((GLfloat)( ROTATION_MIN  + (rand() % this->ROTATION_RANGE))) , glm::vec3(1, 0, 0));
				break;
			}
			case ('-'): // negative rotation
			{
				transformations.top() = rotate(transformations.top(), radians((GLfloat)(rand() % 360)), glm::vec3(0, 1, 0));
				transformations.top() = rotate(transformations.top(), -radians((GLfloat)(ROTATION_MIN + (rand() % this->ROTATION_RANGE))), glm::vec3(1, 0, 0));
				break;
			}
		}
	}
}


void Tree::render(int drawmode)
{

	/* Draw the vertices as GL_POINTS */
	glBindBuffer(GL_ARRAY_BUFFER, this->positionBufferObject);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	/* Bind the  normals */
	glBindBuffer(GL_ARRAY_BUFFER, this->normalsBufferObject);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(2);

	/* Bind the colours */
	glBindBuffer(GL_ARRAY_BUFFER, this->colourBufferObject);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	/* Bind the colours */
	glBindBuffer(GL_ARRAY_BUFFER, this->textureBufferObject);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(3);

	glPointSize(3.f);

	// Enable this line to show model in wireframe
	if (drawmode == 1)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	/* Bind the indexed vertex buffer */
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->elementbuffer);

	for (int i = 0; i < this->stripStarts.size(); i++)
	{

		glDrawElements(GL_TRIANGLE_STRIP, this->stripSizes.at(i), GL_UNSIGNED_INT, (GLvoid*)(this->stripStarts.at(i) * 4)); // *4 needed for pointer because GLuint is 4 bytes
	}

	for (int i = 0; i < this->fanStarts.size(); i++)
	{

		glDrawElements(GL_TRIANGLE_FAN, this->fanSizes.at(i), GL_UNSIGNED_INT, (GLvoid*)(this->fanStarts.at(i) * 4)); // *4 needed for pointer because GLuint is 4 bytes
	}


}