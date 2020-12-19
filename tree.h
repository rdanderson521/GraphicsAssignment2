#ifndef TREE_H
#define TREE_H

#include <stack>
#include <vector>

#include "wrapper_glfw.h"

#include "cylinder.h"

/* Include GLM core and matrix extensions*/
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

using namespace glm;
using namespace std;

// the transformation for an indiviaual "node" of the tree
struct TreeNode
{
	mat4 transformation;
};


class Tree
{
public:

	Tree();

	// generates all of the vertices for a new tree
	void generate(string rule, int recurseLevels = 1);

	// renders the tree
	void render(int drawmode);

private:

	const float BRANCH_SCALE = 0.75;
	const float BRANCH_WIDTH = 0.2, BRANCH_HEIGHT = 1;
	const int ROTATION_MIN = 25, ROTATION_RANGE = 40;

	GLuint positionBufferObject;
	GLuint colourBufferObject;
	GLuint normalsBufferObject;
	GLuint textureBufferObject;
	GLuint elementbuffer;

	GLuint attribute_v_coord;
	GLuint attribute_v_normal;
	GLuint attribute_v_colours;
	GLuint attribute_v_tex;


	int numVertices;

	vector<int> fanStarts, fanSizes, stripStarts, stripSizes;

	Cylinder cylinder;

	vector<TreeNode> nodes;

	// called by generate, needed so a transformation can be passed by ref and so caller of generate doesnt need to pass a mat4
	void generateRecurse(string rule, int recurseLevels, stack<mat4>& transformations);

};

#endif