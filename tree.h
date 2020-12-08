#ifndef TREE_H
#define TREE_H

#ifdef _DEBUG
#pragma comment(lib, "glfw3D.lib")
#pragma comment(lib, "glloadD.lib")
#else
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glload.lib")
#endif
#pragma comment(lib, "opengl32.lib")


#include <stack>
#include <vector>

#include "wrapper_glfw.h"

#include "cubev2.h"


/* Include GLM core and matrix extensions*/
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>



using namespace glm;
using namespace std;


enum NodeType
{
	BRANCH,
	LEAF
};

struct TreeNode
{
	mat4 transformation;
	NodeType type;
};


class Tree
{
public:

	Tree();

	void generate(string rule, int recurseLevels = 1);

	void render(stack<mat4>& model, mat4& view, GLuint renderModelID, GLuint normalMatrixID);

private:

	const float BRANCH_SCALE = 0.8;
	const float BRANCH_WIDTH = 0.2, BRANCH_HEIGHT = 1;
	const int ROTATION_MIN = 15, ROTATION_RANGE = 30;

	GLuint positionBufferObject;
	GLuint colourObject;
	GLuint normalsBufferObject;

	GLuint attribute_v_coord;
	GLuint attribute_v_normal;
	GLuint attribute_v_colours;

	Cubev2 cube;

	vector<TreeNode> nodes;

	void generateRecurse(string rule, int recurseLevels, stack<mat4>& transformations);

};

#endif