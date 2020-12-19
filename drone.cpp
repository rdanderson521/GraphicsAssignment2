#include "drone.h"

#include <stack>

using namespace glm;
using namespace std;


void Drone::init(DroneUniforms uniforms, GLuint tex, GLuint rough, vec3 pos, vec3 orient)
{
	motorBell.makeTube(20, 0.1);
	motorStator.makeCylinder(15);
	cylinder.makeCylinder(8);
	cube.makeCube();

	this->u = uniforms;
	this->frameTexture = tex;
	this->frameRoughnessMap = rough;
	this->pos = pos;
	this->orient = orient;
	this->droneScale = 1.f;
}


void Drone::setLightUniforms(LightsUniformWrapper& uniforms, mat4 view)
{
	stack<mat4> model;
	model.push(mat4(1.0f));

	model.push(model.top());
	{
		this->globalTransformations(model);

		for (int i = 0; i < 4; i++)
		{
			model.push(model.top());
			{
				model.top() = rotate(model.top(), -radians((90 * i) + 45.f), glm::vec3(0, 1, 0));
				model.top() = translate(model.top(), vec3(0.7f, -0.08f, 0.f));

				// calculate and set uniforms for the lights colour and position
				vec3 lightPos = view * model.top() * vec4(1.0f);
				vec3 lightColour;
				if (i > 0 && i < 3)
					lightColour = vec3(1.f, 0.1f, 0.1f);
				else
					lightColour = vec3(0.1f, 1.f, 0.1f);

				uniforms.addPointLight(lightPos, LightMode::OMNI_DIRECTIONAL, lightColour, vec3(1.f, 0.6f, 0.8f));

			}
			model.pop();
		}
		
	}
}


void Drone::globalTransformations(std::stack<glm::mat4>& model)
{

	// global transformations for whole drone
	model.top() = translate(model.top(), this->pos); // translating xyz

	// rotates the model after transforming it so these transformations do not affect the translation
	model.top() = rotate(model.top(), -radians(this->orient.y), glm::vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	model.top() = rotate(model.top(), -radians(this->orient.z), glm::vec3(0, 0, 1)); //rotating in clockwise direction around z-axis
	model.top() = rotate(model.top(), -radians(this->orient.x), glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis

	model.top() = rotate(model.top(), -radians(90.f), glm::vec3(0, 1, 0)); //rotates 90 degrees to align the drone along the axis which make controls easier

	model.top() = scale(model.top(), vec3(this->droneScale, this->droneScale, this->droneScale));//scale equally in all axis
}


void Drone::draw(int drawmode, GLuint programID, mat4 view)
{
	mat3 normalmatrix;

	stack<mat4> model;
	model.push(mat4(1.0f));

	model.push(model.top());
	{	
		
		this->globalTransformations(model);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, frameTexture);
		glUniform1i(u.textureID[programID], 0);

		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, frameRoughnessMap);
		glUniform1i(u.roughnessID[programID], 0);

		glUniform1i(u.useTextureID[programID], 1);
		glUniform1i(u.useRoughnessID[programID], 1);

		// frame top and bottom plates
		model.push(model.top());
		{
			model.top() = translate(model.top(), vec3(0.f, -0.085f, 0.f));
			model.top() = scale(model.top(), framePlateScale);

			glUniformMatrix4fv(u.modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));
			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(u.normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

			cube.drawCube(drawmode);
		}
		model.pop();

		model.push(model.top());
		{
			model.top() = translate(model.top(), vec3(0.f, 0.085f, 0.f));
			model.top() = scale(model.top(), framePlateScale);

			glUniformMatrix4fv(u.modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));
			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(u.normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

			cube.drawCube(drawmode);
		}
		model.pop();

		// arms
		for (int i = 0; i < 4; i++)
		{
			model.push(model.top());
			{
				model.top() = rotate(model.top(), -radians((90 * i) + 45.f), glm::vec3(0, 1, 0));
				model.top() = translate(model.top(), vec3(0.45f, -0.1f, 0.f));
				model.top() = scale(model.top(), frameArmScale);

				glUniformMatrix4fv(u.modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));
				normalmatrix = transpose(inverse(mat3(view * model.top())));
				glUniformMatrix3fv(u.normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

				cube.drawCube(drawmode);
			}
			model.pop();
		}
		// end of frame
		glUniform1i(u.useTextureID[programID], 0);
		glUniform1i(u.useRoughnessID[programID], 0);


		//motors 
		for (int i = 0; i < 4; i++)
		{
			model.push(model.top());
			{

				model.push(model.top());
				{
					model.top() = rotate(model.top(), -radians((90 * i) + 45.f), glm::vec3(0, 1, 0));
					model.top() = translate(model.top(), vec3(0.77f, -0.02f, 0.f));


					// spinning motor bits

					glUniform1f(u.reflectivenessID[programID], motorReflect);
					glUniform4fv(u.colourOverrideID[programID], 1, &motorColour[0]);
					model.push(model.top());
					{
						if (i % 2 == 0)
						{
							model.top() = rotate(model.top(), -radians(motorAngle), glm::vec3(0, 1, 0));
						}
						else
						{
							model.top() = rotate(model.top(), radians(motorAngle), glm::vec3(0, 1, 0));
						}

						model.push(model.top());
						{
							model.top() = translate(model.top(), vec3(0.f, 0.042f, 0.f));
							for (int j = 0; j < 3; j++)
							{
								model.push(model.top());
								{
									model.top() = rotate(model.top(), -radians(120.f * j), glm::vec3(0, 1, 0));

									// propellers
									model.push(model.top());
									{
										if (i % 2 == 0)
										{
											model.top() = rotate(model.top(), -radians(10.f), glm::vec3(1, 0, 0));
										}
										else
										{
											model.top() = rotate(model.top(), radians(10.f), glm::vec3(1, 0, 0));
										}
										model.top() = translate(model.top(), vec3(0.15f, 0.03f, 0.f));
										model.top() = scale(model.top(), vec3(0.3f, 0.01f, 0.05f));

										glUniformMatrix4fv(u.modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));
										normalmatrix = transpose(inverse(mat3(view * model.top())));
										glUniformMatrix3fv(u.normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

										cube.drawCube(drawmode);
									}
									model.pop();

									// strut at top of motor
									model.push(model.top());
									{
										model.top() = translate(model.top(), vec3(0.015f, 0.f, 0.f));
										model.top() = scale(model.top(), motorStrutsScale);

										glUniformMatrix4fv(u.modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));
										normalmatrix = transpose(inverse(mat3(view * model.top())));
										glUniformMatrix3fv(u.normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

										cube.drawCube(drawmode);
									}
									model.pop();
									model.push(model.top());
									{
										model.top() = translate(model.top(), vec3(-0.015f, 0.f, 0.f));
										model.top() = scale(model.top(), motorStrutsScale);

										glUniformMatrix4fv(u.modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));
										normalmatrix = transpose(inverse(mat3(view * model.top())));
										glUniformMatrix3fv(u.normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

										cube.drawCube(drawmode);
									}
									model.pop();
								}
								model.pop();
							}
						}
						model.pop();

						// motor shaft
						glUniform1f(u.reflectivenessID[programID], motorReflect);
						glUniform4fv(u.colourOverrideID[programID], 1, &motorColour[0]);

						model.push(model.top());
						{
							model.top() = translate(model.top(), vec3(0.f, 0.06f, 0.f));
							model.top() = scale(model.top(), motorShaftScale);

							glUniformMatrix4fv(u.modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));
							normalmatrix = transpose(inverse(mat3(view * model.top())));
							glUniformMatrix3fv(u.normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

							cylinder.drawCylinder(drawmode);
						}
						model.pop();

						// motor bell
						model.push(model.top());
						{
							model.top() = scale(model.top(), motorBellScale);
							model.top() = rotate(model.top(), -radians(90.f), glm::vec3(1, 0, 0));

							glUniformMatrix4fv(u.modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));
							normalmatrix = transpose(inverse(mat3(view * model.top())));
							glUniformMatrix3fv(u.normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

							motorBell.drawTube(drawmode);
						}
						model.pop();
					}
					model.pop();

					// motor base 
					glUniform1f(u.reflectivenessID[programID], motorReflect);
					glUniform4fv(u.colourOverrideID[programID], 1, &motorColour[0]);
					model.push(model.top());
					{
						model.top() = translate(model.top(), vec3(0.f, -0.06f, 0.f));
						model.top() = scale(model.top(), vec3(0.12f, 0.01f, 0.04f));

						glUniformMatrix4fv(u.modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));
						normalmatrix = transpose(inverse(mat3(view * model.top())));
						glUniformMatrix3fv(u.normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

						cube.drawCube(drawmode);
					}
					model.pop();

					model.push(model.top());
					{
						model.top() = translate(model.top(), vec3(0.f, -0.06f, 0.f));
						model.top() = scale(model.top(), vec3(0.04f, 0.01f, 0.12f));

						glUniformMatrix4fv(u.modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));
						normalmatrix = transpose(inverse(mat3(view * model.top())));
						glUniformMatrix3fv(u.normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

						cube.drawCube(drawmode);
					}
					model.pop();

					// motor stator
					model.push(model.top());
					{
						model.top() = translate(model.top(), vec3(0.f, -0.015f, 0.f));
						model.top() = scale(model.top(), motorStatorScale);
						//model.top() = rotate(model.top(), -radians(90.f), glm::vec3(1, 0, 0));

						glUniform1f(u.reflectivenessID[programID], motorStatorReflect);
						glUniform4fv(u.colourOverrideID[programID], 1, &motorStatorColour[0]);

						glUniformMatrix4fv(u.modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));
						normalmatrix = transpose(inverse(mat3(view * model.top())));
						glUniformMatrix3fv(u.normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

						motorStator.drawCylinder(drawmode);
					}
					model.pop();

				}
				model.pop();
			}
			model.pop();
		}

		// standoffs
		glUniform1f(u.reflectivenessID[programID], standoffReflect);
		glUniform4fv(u.colourOverrideID[programID], 1, &standoffColour[0]);

		model.push(model.top());
		{
			model.top() = translate(model.top(), vec3(0.45f, 0.00f, 0.12f));
			model.top() = scale(model.top(), standoffScale);
			glUniformMatrix4fv(u.modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));

			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(u.normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

			cylinder.drawCylinder(drawmode);
		}
		model.pop();

		model.push(model.top());
		{
			model.top() = translate(model.top(), vec3(0.2f, 0.00f, 0.12f));
			model.top() = scale(model.top(), standoffScale);

			glUniformMatrix4fv(u.modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));

			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(u.normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

			cylinder.drawCylinder(drawmode);
		}
		model.pop();

		model.push(model.top());
		{
			model.top() = translate(model.top(), vec3(-0.2f, 0.00f, 0.12f));
			model.top() = scale(model.top(), standoffScale);

			glUniformMatrix4fv(u.modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));

			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(u.normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

			cylinder.drawCylinder(drawmode);
		}
		model.pop();

		model.push(model.top());
		{
			model.top() = translate(model.top(), vec3(-0.45f, 0.00f, 0.12f));
			model.top() = scale(model.top(), standoffScale);

			glUniformMatrix4fv(u.modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));

			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(u.normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

			cylinder.drawCylinder(drawmode);
		}
		model.pop();

		model.push(model.top());
		{
			model.top() = translate(model.top(), vec3(0.45f, 0.00f, -0.12f));
			model.top() = scale(model.top(), standoffScale);

			glUniformMatrix4fv(u.modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));

			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(u.normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

			cylinder.drawCylinder(drawmode);
		}
		model.pop();

		model.push(model.top());
		{
			model.top() = translate(model.top(), vec3(0.2f, 0.00f, -0.12f));
			model.top() = scale(model.top(), standoffScale);

			glUniformMatrix4fv(u.modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));

			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(u.normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

			cylinder.drawCylinder(drawmode);
		}
		model.pop();

		model.push(model.top());
		{
			model.top() = translate(model.top(), vec3(-0.2f, 0.00f, -0.12f));
			model.top() = scale(model.top(), standoffScale);

			glUniformMatrix4fv(u.modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));

			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(u.normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

			cylinder.drawCylinder(drawmode);
		}
		model.pop();

		model.push(model.top());
		{
			model.top() = translate(model.top(), vec3(-0.45f, 0.00f, -0.12f));
			model.top() = scale(model.top(), standoffScale);

			glUniformMatrix4fv(u.modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));

			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(u.normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

			cylinder.drawCylinder(drawmode);
		}
		model.pop();

		
	}
	model.pop();
}


void Drone::spinMotor()
{
	this->motorAngle += MOTOR_RATE;
	if (this->motorAngle > 360)
		this->motorAngle -= 360;
}

void Drone::move(glm::vec3 pos, glm::vec3 orient)
{
	this->orient += orient;
	this->orient.x = glm::max(glm::min(30.f, this->orient.x), -30.f);
	this->orient.z = glm::max(glm::min(30.f, this->orient.z), -30.f);

	pos = mat3(rotate(mat4(1.f), -radians(this->orient.y), vec3(0.f, 1.f, 0.f))) * pos;
	this->pos += pos;

}