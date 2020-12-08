#include "tree.h"

#include <iostream>
#include <ctime>

#include "cylinder.h"


Tree::Tree()
{
	this->attribute_v_coord = 0;
	this->attribute_v_colours = 1;
	this->attribute_v_normal = 2;
}


void Tree::generate(string rule, int levels)
{
	this->nodes.clear();
	srand(time(NULL));
	stack<mat4> transformations;
	transformations.push(mat4(1.0f));

	this->generateRecurse(rule, levels, transformations);

	this->cylinder.makeCylinder(15,this->BRANCH_SCALE);

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


void Tree::render(stack<mat4>& model, mat4& view, GLuint renderModelID, GLuint normalMatrixID)
{
	for (auto it = this->nodes.begin(); it != this->nodes.end(); it++)
	{
		model.push(model.top());
		{
			model.top() = model.top() * it->transformation;
			model.top() = scale(model.top(), vec3(this->BRANCH_WIDTH,this->BRANCH_HEIGHT, this->BRANCH_WIDTH));
			model.top() = translate(model.top(), vec3(0, 0.5, 0));

			// Send the model uniform and normal matrix to the currently bound shader,
			glUniformMatrix4fv(renderModelID, 1, GL_FALSE, &(model.top()[0][0]));
			// Recalculate the normal matrix and send to the vertex shader
			mat3 normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(normalMatrixID, 1, GL_FALSE, &normalmatrix[0][0]);
			this->cylinder.drawCylinder(0);
		}
		model.pop();
	}

}