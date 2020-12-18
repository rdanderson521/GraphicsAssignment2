#include "drone.h"

#include <stack>

using namespace glm;
using namespace std;


void Drone::init()
{
	motorBell.makeTube(20, 0.1);
	motorStator.makeCylinder(20);
	cylinder.makeCylinder(8, 0.7);
	cube.makeCube();
}


void Drone::draw(int drawmode)
{
	mat3 normalmatrix;

	stack<mat4> model;
	model.push(mat4(1.0f));

	// This block of code draws the drone
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(x, y, z)); // translating xyz

		// rotates the model after transforming it so these transformations do not affect the translation
		model.top() = rotate(model.top(), -radians(modelAngle_x), glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
		model.top() = rotate(model.top(), -radians(modelAngle_y), glm::vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
		model.top() = rotate(model.top(), -radians(modelAngle_z), glm::vec3(0, 0, 1)); //rotating in clockwise direction around z-axis

		model.top() = rotate(model.top(), -radians(90.f), glm::vec3(0, 1, 0)); //rotates 90 degrees to align the drone along the axis which make controls easier

		model.top() = scale(model.top(), vec3(model_scale, model_scale, model_scale));//scale equally in all axis

		// light sources on drone
		if (lightsOn && programID != SHADOW_PROGRAM)
		{
			for (int i = 0; i < 4; i++)
			{
				/* Draw a small sphere in the lightsource position to visually represent the light source */
				model.push(model.top());
				{
					model.top() = rotate(model.top(), -radians((90 * i) + 45.f), glm::vec3(0, 1, 0));
					model.top() = translate(model.top(), vec3(0.7f, -0.08f, 0.f));
					model.top() = scale(model.top(), vec3(0.02f, 0.02f, 0.02f)); // make a small sphere

					// calculate and set uniforms for the lights colour and position
					vec3 lightPos = view * model.top() * vec4(1.0f);
					vec3 lightColour;
					if (i > 0 && i < 3)
						lightColour = vec3(0.6f, 0.1f, 0.1f);
					else
						lightColour = vec3(0.1f, 0.6f, 0.1f);

					lightsUniformBlock.addPointLight(lightPos, LightMode::OMNI_DIRECTIONAL, lightColour, vec3(0.4f, 0.2f, 0.8f));


					// Recalculate the normal matrix and send the model and normal matrices to the vertex shader																							// Recalculate the normal matrix and send to the vertex shader																								// Recalculate the normal matrix and send to the vertex shader																								// Recalculate the normal matrix and send to the vertex shader																						// Recalculate the normal matrix and send to the vertex shader
					glUniformMatrix4fv(modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));
					normalmatrix = transpose(inverse(mat3(/*view */ model.top())));
					glUniformMatrix3fv(normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

					/* Draw our lightposition sphere  with emit mode on*/
					emitmode = 1;
					glUniform1ui(emitModeID[programID], emitmode);
					sphere.drawSphere(drawmode);
					emitmode = 0;
					glUniform1ui(emitModeID[programID], emitmode);
				}
				model.pop();
			}
		}

		lightsUniformBlock.bind(LIGHT_PARAMS_BINDING);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture[CARBON]);
		glUniform1i(textureID[programID], 0);

		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, roughnessMap[CARBON]);
		glUniform1i(roughnessID[programID], 0);

		glUniform1i(useTextureID[programID], 1);
		glUniform1i(useRoughnessID[programID], 1);

		// frame top and bottom plates
		model.push(model.top());
		{

			model.top() = translate(model.top(), vec3(0.f, -0.085f, 0.f));
			model.top() = scale(model.top(), framePlateScale);

			// set the reflectiveness uniform
			glUniform1f(reflectivenessID[programID], frameReflect);
			// set the colour uniform
			glUniform4fv(colourOverrideID[programID], 1, &frameColour[0]);
			// Send the model uniform and normal matrix to the currently bound shader,
			glUniformMatrix4fv(modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));
			// Recalculate the normal matrix and send to the vertex shader
			normalmatrix = transpose(inverse(mat3(/*view */ model.top())));
			glUniformMatrix3fv(normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

			cube.drawCube(drawmode);
		}
		model.pop();

		model.push(model.top());
		{
			model.top() = translate(model.top(), vec3(0.f, 0.085f, 0.f));
			model.top() = scale(model.top(), framePlateScale);

			// set the reflectiveness uniform
			glUniform1f(reflectivenessID[programID], frameReflect);
			// set the colour uniform
			glUniform4fv(colourOverrideID[programID], 1, &frameColour[0]);
			// Send the model uniform and normal matrix to the currently bound shader,
			glUniformMatrix4fv(modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));
			// Recalculate the normal matrix and send to the vertex shader
			normalmatrix = transpose(inverse(mat3(/*view */ model.top())));
			glUniformMatrix3fv(normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

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

				// set the reflectiveness uniform
				glUniform1f(reflectivenessID[programID], frameReflect);
				// set the colour uniform
				glUniform4fv(colourOverrideID[programID], 1, &frameColour[0]);
				// Send the model uniform and normal matrix to the currently bound shader,
				glUniformMatrix4fv(modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));
				// Recalculate the normal matrix and send to the vertex shader
				normalmatrix = transpose(inverse(mat3(/*view */ model.top())));
				glUniformMatrix3fv(normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

				cube.drawCube(drawmode);
			}
			model.pop();
		}

		glUniform1i(useTextureID[programID], 0);
		glUniform1i(useRoughnessID[programID], 0);


		//motors 
		for (int i = 0; i < 4; i++)
		{
			model.push(model.top());
			{

				model.push(model.top());
				{
					model.top() = rotate(model.top(), -radians((90 * i) + 45.f), glm::vec3(0, 1, 0));
					model.top() = translate(model.top(), vec3(0.77f, -0.02f, 0.f));


					// moving bits
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

										// set the reflectiveness uniform
										glUniform1f(reflectivenessID[programID], motorReflect);
										// set the colour uniform
										glUniform4fv(colourOverrideID[programID], 1, &motorColour[0]);
										// Send the model uniform and normal matrix to the currently bound shader,
										glUniformMatrix4fv(modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));
										// Recalculate the normal matrix and send to the vertex shader
										normalmatrix = transpose(inverse(mat3(/*view */ model.top())));
										glUniformMatrix3fv(normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

										cube.drawCube(drawmode);
									}
									model.pop();
									model.push(model.top());
									{
										model.top() = translate(model.top(), vec3(0.015f, 0.f, 0.f));
										model.top() = scale(model.top(), motorStrutsScale);

										// set the reflectiveness uniform
										glUniform1f(reflectivenessID[programID], motorReflect);
										// set the colour uniform
										glUniform4fv(colourOverrideID[programID], 1, &motorColour[0]);
										// Send the model uniform and normal matrix to the currently bound shader,
										glUniformMatrix4fv(modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));
										// Recalculate the normal matrix and send to the vertex shader
										normalmatrix = transpose(inverse(mat3(/*view */ model.top())));
										glUniformMatrix3fv(normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

										cube.drawCube(drawmode);
									}
									model.pop();
									model.push(model.top());
									{
										model.top() = translate(model.top(), vec3(-0.015f, 0.f, 0.f));
										model.top() = scale(model.top(), motorStrutsScale);

										// set the reflectiveness uniform
										glUniform1f(reflectivenessID[programID], motorReflect);
										// set the colour uniform
										glUniform4fv(colourOverrideID[programID], 1, &motorColour[0]);
										// Send the model uniform and normal matrix to the currently bound shader,
										glUniformMatrix4fv(modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));
										// Recalculate the normal matrix and send to the vertex shader
										normalmatrix = transpose(inverse(mat3(/*view */ model.top())));
										glUniformMatrix3fv(normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

										cube.drawCube(drawmode);
									}
									model.pop();
								}
								model.pop();
							}
						}
						model.pop();

						// motor shaft
						model.push(model.top());
						{
							model.top() = translate(model.top(), vec3(0.f, 0.06f, 0.f));
							model.top() = scale(model.top(), motorShaftScale);
							model.top() = rotate(model.top(), -radians(90.f), glm::vec3(1, 0, 0));

							// set the reflectiveness uniform
							glUniform1f(reflectivenessID[programID], motorReflect);
							// set the colour uniform
							glUniform4fv(colourOverrideID[programID], 1, &motorColour[0]);
							// Send the model uniform and normal matrix to the currently bound shader,
							glUniformMatrix4fv(modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));
							// Recalculate the normal matrix and send to the vertex shader
							normalmatrix = transpose(inverse(mat3(/*view */ model.top())));
							glUniformMatrix3fv(normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

							cylinder.drawTube(drawmode);
						}
						model.pop();

						// motor bell
						model.push(model.top());
						{
							model.top() = scale(model.top(), motorBellScale);
							model.top() = rotate(model.top(), -radians(90.f), glm::vec3(1, 0, 0));

							// set the reflectiveness uniform
							glUniform1f(reflectivenessID[programID], motorReflect);
							// set the colour uniform
							glUniform4fv(colourOverrideID[programID], 1, &motorColour[0]);
							// Send the model uniform and normal matrix to the currently bound shader,
							glUniformMatrix4fv(modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));
							// Recalculate the normal matrix and send to the vertex shader
							normalmatrix = transpose(inverse(mat3(/*view */ model.top())));
							glUniformMatrix3fv(normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

							motorBell.drawTube(drawmode);
						}
						model.pop();
					}
					model.pop();

					// motor base 
					model.push(model.top());
					{
						model.top() = translate(model.top(), vec3(0.f, -0.06f, 0.f));
						model.top() = scale(model.top(), vec3(0.12f, 0.01f, 0.04f));

						// set the reflectiveness uniform
						glUniform1f(reflectivenessID[programID], motorReflect);
						// set the colour uniform
						glUniform4fv(colourOverrideID[programID], 1, &motorColour[0]);
						// Send the model uniform and normal matrix to the currently bound shader,
						glUniformMatrix4fv(modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));
						// Recalculate the normal matrix and send to the vertex shader
						normalmatrix = transpose(inverse(mat3(/*view */ model.top())));
						glUniformMatrix3fv(normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

						cube.drawCube(drawmode);
					}
					model.pop();

					model.push(model.top());
					{
						model.top() = translate(model.top(), vec3(0.f, -0.06f, 0.f));
						model.top() = scale(model.top(), vec3(0.04f, 0.01f, 0.12f));

						// set the reflectiveness uniform
						glUniform1f(reflectivenessID[programID], motorReflect);
						// set the colour uniform
						glUniform4fv(colourOverrideID[programID], 1, &motorColour[0]);
						// Send the model uniform and normal matrix to the currently bound shader,
						glUniformMatrix4fv(modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));
						// Recalculate the normal matrix and send to the vertex shader
						normalmatrix = transpose(inverse(mat3(/*view */ model.top())));
						glUniformMatrix3fv(normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

						cube.drawCube(drawmode);
					}
					model.pop();

					// motor stator
					model.push(model.top());
					{
						model.top() = translate(model.top(), vec3(0.f, -0.015f, 0.f));
						model.top() = scale(model.top(), motorStatorScale);
						model.top() = rotate(model.top(), -radians(90.f), glm::vec3(1, 0, 0));

						// set the reflectiveness uniform
						glUniform1f(reflectivenessID[programID], motorStatorReflect);
						// set the colour uniform
						glUniform4fv(colourOverrideID[programID], 1, &motorStatorColour[0]);
						// Send the model uniform and normal matrix to the currently bound shader,
						glUniformMatrix4fv(modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));
						// Recalculate the normal matrix and send to the vertex shader
						normalmatrix = transpose(inverse(mat3(/*view */ model.top())));
						glUniformMatrix3fv(normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

						motorStator.drawCylinder(drawmode);
					}
					model.pop();

				}
				model.pop();
			}
			model.pop();
		}

		// standoffs
		{

			model.push(model.top());
			{
				model.top() = translate(model.top(), vec3(0.45f, 0.00f, 0.12f));
				model.top() = scale(model.top(), standoffScale);
				model.top() = rotate(model.top(), -radians(90.f), glm::vec3(1, 0, 0));

				// set the reflectiveness uniform
				glUniform1f(reflectivenessID[programID], standoffReflect);
				// set the colour uniform
				glUniform4fv(colourOverrideID[programID], 1, &standoffColour[0]);
				// Send the model uniform and normal matrix to the currently bound shader,
				glUniformMatrix4fv(modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));
				// Recalculate the normal matrix and send to the vertex shader
				normalmatrix = transpose(inverse(mat3(/*view */ model.top())));
				glUniformMatrix3fv(normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

				/* Draw our cube*/
				cylinder.drawCylinder(drawmode);
			}
			model.pop();

			model.push(model.top());
			{
				model.top() = translate(model.top(), vec3(0.2f, 0.00f, 0.12f));
				model.top() = scale(model.top(), standoffScale);
				model.top() = rotate(model.top(), -radians(90.f), glm::vec3(1, 0, 0));

				// Send the model uniform and normal matrix to the currently bound shader,
				glUniformMatrix4fv(modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));

				// Recalculate the normal matrix and send to the vertex shader
				normalmatrix = transpose(inverse(mat3(/*view */ model.top())));
				glUniformMatrix3fv(normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

				/* Draw our cube*/
				tube.drawTube(drawmode);
			}
			model.pop();

			model.push(model.top());
			{
				model.top() = translate(model.top(), vec3(-0.2f, 0.00f, 0.12f));
				model.top() = scale(model.top(), standoffScale);
				model.top() = rotate(model.top(), -radians(90.f), glm::vec3(1, 0, 0));

				// Send the model uniform and normal matrix to the currently bound shader,
				glUniformMatrix4fv(modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));

				// Recalculate the normal matrix and send to the vertex shader
				normalmatrix = transpose(inverse(mat3(/*view */ model.top())));
				glUniformMatrix3fv(normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

				/* Draw our cube*/
				tube.drawTube(drawmode);
			}
			model.pop();

			model.push(model.top());
			{
				model.top() = translate(model.top(), vec3(-0.45f, 0.00f, 0.12f));
				model.top() = scale(model.top(), standoffScale);
				model.top() = rotate(model.top(), -radians(90.f), glm::vec3(1, 0, 0));

				// Send the model uniform and normal matrix to the currently bound shader,
				glUniformMatrix4fv(modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));

				// Recalculate the normal matrix and send to the vertex shader
				normalmatrix = transpose(inverse(mat3(/*view */ model.top())));
				glUniformMatrix3fv(normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

				/* Draw our cube*/
				tube.drawTube(drawmode);
			}
			model.pop();

			model.push(model.top());
			{
				model.top() = translate(model.top(), vec3(0.45f, 0.00f, -0.12f));
				model.top() = scale(model.top(), standoffScale);
				model.top() = rotate(model.top(), -radians(90.f), glm::vec3(1, 0, 0));

				// Send the model uniform and normal matrix to the currently bound shader,
				glUniformMatrix4fv(modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));

				// Recalculate the normal matrix and send to the vertex shader
				normalmatrix = transpose(inverse(mat3(/*view */ model.top())));
				glUniformMatrix3fv(normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

				/* Draw our cube*/
				tube.drawTube(drawmode);
			}
			model.pop();

			model.push(model.top());
			{
				model.top() = translate(model.top(), vec3(0.2f, 0.00f, -0.12f));
				model.top() = scale(model.top(), standoffScale);
				model.top() = rotate(model.top(), -radians(90.f), glm::vec3(1, 0, 0));

				// Send the model uniform and normal matrix to the currently bound shader,
				glUniformMatrix4fv(modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));

				// Recalculate the normal matrix and send to the vertex shader
				normalmatrix = transpose(inverse(mat3(/*view */ model.top())));
				glUniformMatrix3fv(normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

				/* Draw our cube*/
				tube.drawTube(drawmode);
			}
			model.pop();

			model.push(model.top());
			{
				model.top() = translate(model.top(), vec3(-0.2f, 0.00f, -0.12f));
				model.top() = scale(model.top(), standoffScale);
				model.top() = rotate(model.top(), -radians(90.f), glm::vec3(1, 0, 0));

				// Send the model uniform and normal matrix to the currently bound shader,
				glUniformMatrix4fv(modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));

				// Recalculate the normal matrix and send to the vertex shader
				normalmatrix = transpose(inverse(mat3(/*view */ model.top())));
				glUniformMatrix3fv(normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

				/* Draw our cube*/
				tube.drawTube(drawmode);
			}
			model.pop();

			model.push(model.top());
			{
				model.top() = translate(model.top(), vec3(-0.45f, 0.00f, -0.12f));
				model.top() = scale(model.top(), standoffScale);
				model.top() = rotate(model.top(), -radians(90.f), glm::vec3(1, 0, 0));

				// Send the model uniform and normal matrix to the currently bound shader,
				glUniformMatrix4fv(modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));

				// Recalculate the normal matrix and send to the vertex shader
				normalmatrix = transpose(inverse(mat3(/*view */ model.top())));
				glUniformMatrix3fv(normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

				/* Draw our cube*/
				tube.drawTube(drawmode);
			}
			model.pop();

		}
	}
	model.pop();
}