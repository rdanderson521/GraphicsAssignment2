
/* Link to static libraries, could define these as linker inputs in the project settings instead
if you prefer */
#ifdef _DEBUG
#pragma comment(lib, "glfw3D.lib")
#pragma comment(lib, "glloadD.lib")
#else
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glload.lib")
#endif
#pragma comment(lib, "opengl32.lib")

/* Include the header to the GLFW wrapper class which
   also includes the OpenGL extension initialisation*/
#include "wrapper_glfw.h"
#include <iostream>
#include <stack>
#include <chrono>

   /* Include GLM core and matrix extensions*/
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

// Include headers for our objects
#include "sphere.h"
#include "cubev2.h"
#include "tube.h"
#include "tree.h"
#include "terrain_object.h"

#include "texture_loader.h"

/* Define buffer object indices */
GLuint elementbuffer;

enum programID
{
	MAIN_PROGRAM,
	TERRAIN_PROGRAM,
	SHADOW_PROGRAM,
	NUM_PROGRAMS
};

GLuint programs[NUM_PROGRAMS];		/* Identifier for the shader prgoram */
GLuint vao;			/* Vertex array (Containor) object. This is the index of the VAO that will be the container for
					   our buffer objects */


// globals for shadow mapping
//GLuint shadowProgram;	// shader program for shadow rendering
GLuint depthMapFBO; // depth map for shadows
GLuint depthMap; // idx for texture for shadow map
const unsigned int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;
//GLuint shadowsModelID, shadowsLightSpaceMatrixID;

GLuint colourmode;	/* Index of a uniform to switch the colour mode in the vertex shader
					  I've included this to show you how to pass in an unsigned integer into
					  your vertex shader. */
GLuint emitmode;
GLuint attenuationmode;

/* Position and view globals */
GLfloat angle_x, angle_inc_x, x, model_scale, z, y, vx, vy, vz;
GLfloat angle_y, angle_inc_y, angle_z, angle_inc_z;
GLuint drawmode;			// Defines drawing mode of sphere as points, lines or filled polygons
GLfloat speed;				// movement increment
GLfloat motorAngle;				
GLfloat motorAngleInc;				
bool lightsOn;


// globals for animation
GLfloat modelAngle_x, modelAngle_y, modelAngle_z, modelAngleChange;
GLfloat moveX, moveY, moveZ;


const int numTerrainTextures = 4;
const int maxNumLights = 100;

/* Uniforms*/
GLuint modelID[NUM_PROGRAMS], viewID[NUM_PROGRAMS], projectionID[NUM_PROGRAMS], normalMatrixID[NUM_PROGRAMS], viewPosID[NUM_PROGRAMS];
GLuint colourModeID[NUM_PROGRAMS], emitModeID[NUM_PROGRAMS];
GLuint colourOverrideID[NUM_PROGRAMS], reflectivenessID[NUM_PROGRAMS], numLightsID[NUM_PROGRAMS];
GLuint lightSpaceMatrixID[NUM_PROGRAMS], shadowMapID[NUM_PROGRAMS];
GLuint textureID[NUM_PROGRAMS], terrainTextureID[numTerrainTextures], terrainTextureThresholdID[numTerrainTextures], useTextureID[NUM_PROGRAMS];
GLuint lightPosID[NUM_PROGRAMS][maxNumLights];
GLuint lightColourID[NUM_PROGRAMS][maxNumLights];
GLuint lightModeID[NUM_PROGRAMS][maxNumLights];
GLuint attenuationModeID[NUM_PROGRAMS][maxNumLights];
GLuint lightAuttentiationID[NUM_PROGRAMS][maxNumLights];


vec3 directionalLightDir = normalize(vec3(1.f, -1.f, 2.f));
int numLights;


int controlMode;

GLfloat aspect_ratio;		/* Aspect ratio of the window defined in the reshape callback*/
int windowWidth, windowHeight;
GLuint numspherevertices;




/* Global instances of our objects */
Tube tube;
Tube motorBell, motorStator, motorShaft;
Cubev2 cube;
Sphere sphere;
Tree tree;
terrain_object* terrain;

std::vector<vec3> treeLocations;

enum textures
{
	SAND,
	GRASS,
	DIRT,
	ROCK,
	BARK,
	totalNumTextures
};

// texture IDs
GLuint texture[totalNumTextures];
GLuint normalMap[totalNumTextures];

using namespace std;
using namespace glm;

/*
This function is called before entering the main rendering loop.
Use it for all your initialisation stuff
*/
void init(GLWrapper* glw)
{
	/* Set the object transformation controls to their initial values */
	speed = 0.025f;
	x = 0.05f;
	y = 0;
	z = 0;
	vx = 0; vx = 0, vz = 4.f;
	//light_x = 0; light_y = 1; light_z = 0;
	angle_x = angle_y = angle_z = 0;
	angle_inc_x = angle_inc_y = angle_inc_z = 0;
	model_scale = 1.f;
	aspect_ratio = 1.3333f;
	colourmode = 1; 
	emitmode = 0;
	attenuationmode = 1; // Attenuation is on by default
	motorAngle = 0;
	numLights = 0;
	controlMode = 2;

	//control mode 2 defaults
	modelAngleChange = 2.f;
	angle_inc_x = 0;
	angle_inc_y = 0;
	angle_inc_z = 0;
	angle_x = 0;
	angle_y = 0;
	angle_z = 0;
	modelAngle_x = 0;
	modelAngle_y = 0;
	modelAngle_z = 0;
	model_scale = 1.f;
	x = 0;
	y = 0;
	z = 4;
	lightsOn = true;


	/* Load and build the vertex and fragment shaders */
	try
	{
		programs[MAIN_PROGRAM] = glw->LoadShader("shaders\\poslight.vert", "shaders\\poslight.frag");
	}
	catch (exception& e)
	{
		cout << "Caught exception: " << e.what() << endl;
		cin.ignore();
		exit(0);
	}

	/* Load and build the vertex and fragment shaders */
	try
	{
		programs[TERRAIN_PROGRAM] = glw->LoadShader("shaders\\terrain.vert", "shaders\\terrain.frag");
	}
	catch (exception& e)
	{
		cout << "Caught exception: " << e.what() << endl;
		cin.ignore();
		exit(0);
	}

	/* Load and build the vertex and fragment shaders */
	try
	{
		programs[SHADOW_PROGRAM] = glw->LoadShader("shaders\\shadows.vert", "shaders\\shadows.frag");
	}
	catch (exception& e)
	{
		cout << "Caught exception: " << e.what() << endl;
		cin.ignore();
		exit(0);
	}

	// generates the frame buffer for the shadow map
	glGenFramebuffers(1, &depthMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

	// generates the texture for the shadow map
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 10.f, 10.f, 10.f, 10.f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// attaches the texture to the frame buffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "frame buffer invalid" << endl;
		cin.ignore();
		exit(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	


	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	// Create the vertex array object and make it current
	glBindVertexArray(vao);

	for (int i = 0; i < NUM_PROGRAMS; i++)
	{
		modelID[i] = glGetUniformLocation(programs[i], "model");
		lightSpaceMatrixID[i] = glGetUniformLocation(programs[i], "lightSpaceMatrix");

		if (i != SHADOW_PROGRAM)
		{
			colourModeID[i] = glGetUniformLocation(programs[i], "colourMode");
			emitModeID[i] = glGetUniformLocation(programs[i], "emitMode");
			viewID[i] = glGetUniformLocation(programs[i], "view");
			projectionID[i] = glGetUniformLocation(programs[i], "projection");
			normalMatrixID[i] = glGetUniformLocation(programs[i], "normalMatrix");
			for (int j = 0; j < maxNumLights; j++)
			{
				std::string str = "lightPos[" + std::to_string(j) + "]";
				lightPosID[i][j] = glGetUniformLocation(programs[i], str.c_str());

				str = "lightColour[" + std::to_string(j) + "]";
				lightColourID[i][j] = glGetUniformLocation(programs[i], str.c_str());

				str = "lightMode[" + std::to_string(j) + "]";
				lightModeID[i][j] = glGetUniformLocation(programs[i], str.c_str());

				str = "attenuationMode[" + std::to_string(j) + "]";
				attenuationModeID[i][j] = glGetUniformLocation(programs[i], str.c_str());
			}
			numLightsID[i] = glGetUniformLocation(programs[i], "numLights");
			viewPosID[i] = glGetUniformLocation(programs[i], "viewPos");
			colourOverrideID[i] = glGetUniformLocation(programs[i], "colourOverride");
			reflectivenessID[i] = glGetUniformLocation(programs[i], "reflectiveness");

			shadowMapID[i] = glGetUniformLocation(programs[i], "shadowMap");

			useTextureID[i] = glGetUniformLocation(programs[i], "useTex");
			if (i == MAIN_PROGRAM)
			{
				textureID[i] = glGetUniformLocation(programs[i], "tex");
			}
			else if (i == TERRAIN_PROGRAM)
			{
				for (int j = 0; j < numTerrainTextures; j++)
				{
					std::string str = "tex[" + std::to_string(j) + "]";
					terrainTextureID[j] = glGetUniformLocation(programs[i], str.c_str());
					str = "texThres[" + std::to_string(j) + "]";
					terrainTextureThresholdID[j] = glGetUniformLocation(programs[i], str.c_str());
				}
			}

		}

		

	}

	/* create our sphere and cube objects */

	sphere.makeSphere(20, 20);
	tube.makeTube(15, 0.1);
	motorBell.makeTube(40, 0.1);
	motorStator.makeTube(40, 0.85);
	motorShaft.makeTube(40, 0.7);
	cube.makeCube();
	tree.generate("F[[-F]F[+F]]",4);

	terrain = new terrain_object(4, 1.f, 2.f);
	terrain->createTerrain(100, 100, 20.f, 20.f);
	terrain->setColourBasedOnHeight();
	terrain->createObject();

	for (int i = 0; i < 20; i++)
	{
		int treeX = (rand() % 20) - 10, treeZ = (rand() % 20) - 10;
		int treeY = terrain->heightAtPosition(treeX, treeZ);
		treeLocations.push_back(vec3(treeX, treeY, treeZ));
	}
	
	for (int i = 0; i < totalNumTextures; i++)
	{
		switch (i)
		{
			case(BARK):
			{
				texture[i] = loadTexture("textures/Bark_004_basecolor.jpg");
				normalMap[i] = loadTexture("textures/Bark_004_normal.jpg");
				break;
			}
			case(SAND):
			{
				texture[i] = loadTexture("textures/Sand_007_basecolor.jpg");
				normalMap[i] = loadTexture("textures/Sand_007_normal.jpg");
				break;
			}
			case(GRASS):
			{
				texture[i] = loadTexture("textures/Stylized_Grass_001_basecolor.jpg");
				normalMap[i] = loadTexture("textures/Stylized_Grass_001_normal.jpg");
				break;
			}
			case(DIRT):
			{
				texture[i] = loadTexture("textures/Ground_wet_003_basecolor.jpg");
				normalMap[i] = loadTexture("textures/Ground_wet_003_normal.jpg");
				break;
			case(ROCK):
			{
				texture[i] = loadTexture("textures/Rock_028_COLOR.jpg");
				normalMap[i] = loadTexture("textures/Rock_028_NORM.jpg");
				break;
			}
			}
		}
	}
	

	// print instructions
	cout << endl <<
		"####\\/ Drone Simulator \\/####" << endl << 
		endl <<
		"##### Modes #####" << endl <<
		"[1]: View Mode" << endl <<
		"[2]: Fly Mode (default)" << endl << 
		endl <<
		"##### View Mode #####" << endl <<
		"[Q]: Zoom out" << endl <<
		"[E]: Zoom in" << endl <<
		"[W]: Pan up" << endl <<
		"[S]: Pan down" << endl <<
		"[A]: Increase pan speed left" << endl <<
		"[D]: Increase pan speed right" << endl << 
		"[R]: Increase propeller speed" << endl <<
		"[T]: Decrease propeller speed right" << endl <<
		endl <<
		"##### Fly Mode #####" << endl <<
		"[Q]: Fly down/ land" << endl <<
		"[E]: Fly up/ takeoff" << endl <<
		"[W]: Fly forward" << endl <<
		"[S]: Fly backward" << endl <<
		"[A]: Fly left" << endl <<
		"[D]: Fly right" << endl <<
		"The green lights on the drone are forward and red are back." << endl <<
		"The drone has to be a bit off the ground before you can start moving around" << endl << 
		endl <<
		"##### General Buttons #####" << endl <<
		"[F] Turn lights on the drone on/off (on by default)" << endl <<
		"[,] Switch between draw modes to see the triangles or vertices" << endl;

}

void render(mat4& view, GLuint programID)
{

	mat3 normalmatrix;

	vec3 framePlateScale = vec3(1.f, 0.015f, 0.3f);
	vec3 frameArmScale = vec3(0.8f, 0.03f, 0.15f);
	vec3 standoffScale = vec3(0.025f, 0.17f, 0.025f);

	vec3 motorBellScale = vec3(0.15f, 0.085f, 0.15f);
	vec3 motorStatorScale = vec3(0.125f, 0.08f, 0.125f);
	vec3 motorShaftScale = vec3(0.025f, 0.085f, 0.025f);
	vec3 motorStrutsScale = vec3(0.011f, 0.011f, 0.14f);
	vec3 groundPlaneScale = vec3(20.f, 0.0001f, 20.f);

	vec4 frameColour = vec4(0.20f, 0.20f, 0.20f, 1.f);
	vec4 treeColour = vec4(0.50f, 0.25f, 0.f, 1.f);
	vec4 motorColour = vec4(0.60f, 0.60f, 0.60f, 1.f);
	vec4 motorStatorColour = vec4(0.88f, 0.44f, 0.f, 1.f);
	vec4 standoffColour = vec4(1.f, 0.f, 0.f, 1.f);
	vec3 groundPlaneColour = vec3(0.8f, 0.8f, 0.8f);

	GLfloat frameReflect = 0.f;
	GLfloat motorReflect = 8.f;
	GLfloat motorStatorReflect = 2.f;
	GLfloat standoffReflect = 1.f;

	// Define our model transformation in a stack and 
	// push the identity matrix onto the stack
	stack<mat4> model;
	model.push(mat4(1.0f));

	// This block of code draws the drone
	model.push(model.top());
	{
		// Define the global model transformations (rotate and scale). Note, we're not modifying thel ight source position
		//model.top() = rotate(model.top(), -radians(angle_x), glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
		//model.top() = rotate(model.top(), -radians(angle_y), glm::vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
		//model.top() = rotate(model.top(), -radians(angle_z), glm::vec3(0, 0, 1)); //rotating in clockwise direction around z-axis
		model.top() = translate(model.top(), vec3(x, y, z)); // translating xyz

		// rotates the model after transforming it so these transformations do not affect the translation
		model.top() = rotate(model.top(), -radians(modelAngle_x), glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
		model.top() = rotate(model.top(), -radians(modelAngle_y), glm::vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
		model.top() = rotate(model.top(), -radians(modelAngle_z), glm::vec3(0, 0, 1)); //rotating in clockwise direction around z-axis

		model.top() = rotate(model.top(), -radians(90.f), glm::vec3(0, 1, 0)); //rotates 90 degrees to align the drone along the axis which make controls easier

		model.top() = scale(model.top(), vec3(model_scale, model_scale, model_scale));//scale equally in all axis
		
		// light sources on drone
		if (lightsOn)
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
					glUniform4fv(lightPosID[programID][numLights], 1, &lightPos[0]);
					glUniform3fv(lightColourID[programID][numLights], 1, &lightColour[0]);
					glUniform1ui(lightModeID[programID][numLights], 1);
					glUniform1ui(attenuationModeID[programID][numLights], 0);
					glUniform1ui(numLightsID[programID], ++numLights);

					// Recalculate the normal matrix and send the model and normal matrices to the vertex shader																							// Recalculate the normal matrix and send to the vertex shader																								// Recalculate the normal matrix and send to the vertex shader																								// Recalculate the normal matrix and send to the vertex shader																						// Recalculate the normal matrix and send to the vertex shader
					glUniformMatrix4fv(modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));
					normalmatrix = transpose(inverse(mat3(view * model.top())));
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
			normalmatrix = transpose(inverse(mat3(view * model.top())));
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
			normalmatrix = transpose(inverse(mat3(view * model.top())));
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
				normalmatrix = transpose(inverse(mat3(view * model.top())));
				glUniformMatrix3fv(normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

				cube.drawCube(drawmode);
			}
			model.pop();
		}


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
										model.top() = scale(model.top(), vec3(0.3f,0.01f,0.05f));

										// set the reflectiveness uniform
										glUniform1f(reflectivenessID[programID], motorReflect);
										// set the colour uniform
										glUniform4fv(colourOverrideID[programID], 1, &motorColour[0]);
										// Send the model uniform and normal matrix to the currently bound shader,
										glUniformMatrix4fv(modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));
										// Recalculate the normal matrix and send to the vertex shader
										normalmatrix = transpose(inverse(mat3(view * model.top())));
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
										normalmatrix = transpose(inverse(mat3(view * model.top())));
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
										normalmatrix = transpose(inverse(mat3(view * model.top())));
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
							normalmatrix = transpose(inverse(mat3(view * model.top())));
							glUniformMatrix3fv(normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

							motorShaft.drawTube(drawmode);
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
							normalmatrix = transpose(inverse(mat3(view * model.top())));
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
						normalmatrix = transpose(inverse(mat3(view * model.top())));
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
						normalmatrix = transpose(inverse(mat3(view * model.top())));
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
						normalmatrix = transpose(inverse(mat3(view * model.top())));
						glUniformMatrix3fv(normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

						motorStator.drawTube(drawmode);
					}
					model.pop();

				}
				model.pop();
			}
			model.pop();
		}


		model.push(model.top());
		{
		}
		model.pop();

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
				normalmatrix = transpose(inverse(mat3(view * model.top())));
				glUniformMatrix3fv(normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

				/* Draw our cube*/
				tube.drawTube(drawmode);
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
				normalmatrix = transpose(inverse(mat3(view * model.top())));
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
				normalmatrix = transpose(inverse(mat3(view * model.top())));
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
				normalmatrix = transpose(inverse(mat3(view * model.top())));
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
				normalmatrix = transpose(inverse(mat3(view * model.top())));
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
				normalmatrix = transpose(inverse(mat3(view * model.top())));
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
				normalmatrix = transpose(inverse(mat3(view * model.top())));
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
				normalmatrix = transpose(inverse(mat3(view * model.top())));
				glUniformMatrix3fv(normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

				/* Draw our cube*/
				tube.drawTube(drawmode);
			}
			model.pop();

		}
	}
	model.pop();

	// trees
	model.push(model.top());
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture[BARK]);
		glUniform1i(textureID[programID], 0);

		// set the reflectiveness uniform
		glUniform1f(reflectivenessID[programID], frameReflect);
		// set the colour uniform
		glUniform4fv(colourOverrideID[programID], 1, &treeColour[0]);

		glUniform1i(useTextureID[programID], 1);

		//std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

		for (int i = 0; i < treeLocations.size(); i++)
		{
			model.push(model.top());
			{
				model.top() = translate(model.top(), treeLocations.at(i));
				
				// Send the model uniform and normal matrix to the currently bound shader,
				glUniformMatrix4fv(modelID[programID], 1, GL_FALSE, &(model.top()[0][0]));
				// Recalculate the normal matrix and send to the vertex shader
				normalmatrix = transpose(inverse(mat3(view * model.top())));
				glUniformMatrix3fv(normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

				tree.render(drawmode);

				
			}
			model.pop();
		}

		glUniform1i(useTextureID[programID], 0);
	}
	model.pop();
}

void renderTerrain(mat4& view, GLuint programID)
{
	mat4 model = mat4(1.0f);

	for (int i = 0; i < numTerrainTextures; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		int textureIdx = SAND + i;
		
		glBindTexture(GL_TEXTURE_2D, texture[textureIdx]);
		glUniform1i(terrainTextureID[i], i);
		glUniform1f(terrainTextureThresholdID[i], (i * 1.f) + 0.01f);
	}


	glUniform1i(useTextureID[programID], 1);


	// set the reflectiveness uniform
	glUniform1f(reflectivenessID[programID], 0.f);
	// set the colour uniform
	glUniform1ui(colourModeID[programID], 0);
	// Send the model uniform and normal matrix to the currently bound shader,
	glUniformMatrix4fv(modelID[programID], 1, GL_FALSE, &(model[0][0]));
	// Recalculate the normal matrix and send to the vertex shader
	mat3 normalmatrix = transpose(inverse(mat3(/*view */ model)));
	glUniformMatrix3fv(normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

	terrain->drawObject(drawmode);

	glUniform1i(useTextureID[programID], 0);
}

void resetLights()
{
	numLights = 0;
	vec4 temp(0.f);
	for (int i = 0; i < NUM_PROGRAMS; i++)
	{
		for (int j = 0; j < maxNumLights; j++)
		{
			glUniform4fv(lightPosID[i][numLights], 1, &temp[0]);
			glUniform3fv(lightColourID[i][numLights], 1, &temp[0]);
		}
		glUniform1ui(numLightsID[i], 0);
	}
	
}

void setDirectionalLightUniforms(GLuint programID)
{
	vec3 lightColour = vec3(1.f);
	glUniform4fv(lightPosID[programID][numLights], 1, &directionalLightDir[0]);
	glUniform1ui(lightModeID[programID][numLights], 0);
	glUniform3fv(lightColourID[programID][numLights], 1, &lightColour[0]);
	glUniform1ui(attenuationModeID[programID][numLights], 0);
	glUniform1ui(numLightsID[programID], ++numLights);
}

/* Called to update the display. Note that this function is called in the event loop in the wrapper
   class because we registered display as a callback function */
void display()
{
	// Define the normal matrix
	mat3 normalmatrix;

	// Projection matrix : 60� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	mat4 renderProjection;

	// Camera matrix
	mat4 renderView;


	renderProjection = perspective(radians(75.f), aspect_ratio, 0.1f, 20.f);

	if (controlMode == 1)
	{
		renderView = lookAt(
			vec3(0, 0, -4), // Camera is at (0,0,4), in World Space
			vec3(0, 0, 0), // and looks at the origin
			vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);

		renderView = rotate(renderView, -angle_x, vec3(1, 0, 0));
		renderView = rotate(renderView, radians(angle_y), vec3(0, 1, 0));

	}
	else if (controlMode == 2)
	{
		GLfloat temp = x / z;
		if (abs(x) < 0.01 || abs(z) < 0.01)
			temp = 0;

		renderView = lookAt(
			vec3(0, 7, 0), // Camera is at (0,0,4), in World Space
			vec3(x, y, z), // and looks at the origin
			vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);
	}

	vec3 lightDirNormalised = normalize(directionalLightDir);

	mat4 shadowView = glm::lookAt(
		-directionalLightDir,
		vec3(0.f, 0.f, 0.f),
		vec3(0.0f, 1.0f, 0.0f));

	mat3 lightVectors = shadowView;

	// below code gets the points in the cameras frustum in world 
	vector<vec4> cubeNDC;
	cubeNDC.push_back(vec4(-1.0f, -1.0f, -1.0f, 1.0f));
	cubeNDC.push_back(vec4(1.0f, -1.0f, -1.0f, 1.0f));
	cubeNDC.push_back(vec4(1.0f, -1.0f, 1.0f, 1.0f));
	cubeNDC.push_back(vec4(-1.0f, -1.0f, 1.0f, 1.0f));
	cubeNDC.push_back(vec4(-1.0f, 1.0f, -1.0f, 1.0f));
	cubeNDC.push_back(vec4(1.0f, 1.0f, -1.0f, 1.0f));
	cubeNDC.push_back(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	cubeNDC.push_back(vec4(-1.0f, 1.0f, 1.0f, 1.0f));

	mat4 viewProjectionMatrixInverse = inverse(renderProjection * renderView);

	vector <vec4> cameraFrustum;
	for (vec4 vertex : cubeNDC) {
		vec4 vertexTransformed = viewProjectionMatrixInverse * vertex;
		vertexTransformed /= vertexTransformed.w;
		cameraFrustum.push_back(vertexTransformed);
	}

	vec3 minBound, maxBound;
	for (int i = 0; i < 3; i++)
	{
		bool first = true;
		for (vec3 vertex : cameraFrustum)
		{
			if (first)
			{
				minBound[i] = maxBound[i] = dot(vertex, lightVectors[i]);
				first = false;
			}
			else
			{
				minBound[i] = std::min(minBound[i], dot(vertex, lightVectors[i]));
				maxBound[i] = std::max(maxBound[i], dot(vertex, lightVectors[i]));
			}
		}
	}

	// render shadow maps
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glUseProgram(programs[SHADOW_PROGRAM]);

	resetLights();

	vec3 bounds = maxBound - minBound;

	mat4 shadowProjection = ortho(-0.5f * bounds.x, 0.5f * bounds.x, -0.5f * bounds.y, 0.5f * bounds.y, -10.f, bounds.z);


	mat4 lightSpace = shadowProjection * shadowView;


	glUniformMatrix4fv(lightSpaceMatrixID[SHADOW_PROGRAM], 1, GL_FALSE, &lightSpace[0][0]);

	render(shadowView,SHADOW_PROGRAM);
	renderTerrain(shadowView,SHADOW_PROGRAM);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);
	
	// render actual view

	glViewport(0, 0, windowWidth, windowHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Define the background colour */
	glClearColor(0.2f, 0.5f, 1.0f, 1.0f);

	/* Clear the colour and frame buffers */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Enable depth test  */
	glEnable(GL_DEPTH_TEST);

	/* Make the compiled shader program current */
	glUseProgram(programs[MAIN_PROGRAM]);

	resetLights();
	setDirectionalLightUniforms(MAIN_PROGRAM);

	// Send our projection and view uniforms to the currently bound shader
	// I do that here because they are the same for all objects
	glUniform1ui(colourModeID[MAIN_PROGRAM], colourmode);
	glUniformMatrix4fv(viewID[MAIN_PROGRAM], 1, GL_FALSE, &renderView[0][0]);
	glUniformMatrix4fv(projectionID[MAIN_PROGRAM], 1, GL_FALSE, &renderProjection[0][0]);
	glUniformMatrix4fv(lightSpaceMatrixID[MAIN_PROGRAM], 1, GL_FALSE, &lightSpace[0][0]);
	
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glUniform1i(shadowMapID[MAIN_PROGRAM], 5);


	render(renderView,MAIN_PROGRAM);
	//renderTerrain(renderView, MAIN_PROGRAM);

	glUseProgram(programs[TERRAIN_PROGRAM]);

	resetLights();
	setDirectionalLightUniforms(TERRAIN_PROGRAM);

	// Send our projection and view uniforms to the currently bound shader
	// I do that here because they are the same for all objects
	glUniform1ui(colourModeID[TERRAIN_PROGRAM], colourmode);
	glUniformMatrix4fv(viewID[TERRAIN_PROGRAM], 1, GL_FALSE, &renderView[0][0]);
	glUniformMatrix4fv(projectionID[TERRAIN_PROGRAM], 1, GL_FALSE, &renderProjection[0][0]);
	glUniformMatrix4fv(lightSpaceMatrixID[TERRAIN_PROGRAM], 1, GL_FALSE, &lightSpace[0][0]);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glUniform1i(shadowMapID[TERRAIN_PROGRAM], 5);

	renderTerrain(renderView,TERRAIN_PROGRAM);

	glDisableVertexAttribArray(0);
	glUseProgram(0);

	/* Modify our animation variables */
	

	GLfloat minmaxXZ = 9.5f;
	GLfloat maxY = 15.f;
	GLfloat minY = terrain->heightAtPosition(x,z) + 0.2f;
	GLfloat minYFly = terrain->heightAtPosition(x, z)+ 0.2f;
	
	if (controlMode == 1)
	{
		angle_y += angle_inc_y;
		motorAngle += motorAngleInc;
	}
	else if (controlMode == 2)
	{
		if (moveY > 0 && y < maxY)
		{
			y += moveY;
		}
		else if (moveY < 0 && y > minY)
		{
			y += moveY;
		}

		if (moveZ > 0 && z < minmaxXZ && y > minYFly)
		{
			z += moveZ;
			modelAngle_x = glm::max(-30.f, modelAngle_x - modelAngleChange);
		}
		else if (moveZ < 0 && z > -minmaxXZ && y > minYFly)
		{
			z += moveZ;
			modelAngle_x = glm::min(30.f, modelAngle_x + modelAngleChange);
		}
		else
		{
			if (modelAngle_x > 0)
				modelAngle_x -= modelAngleChange;
			if (modelAngle_x < 0)
				modelAngle_x += modelAngleChange;
		}


		if (moveX > 0 && x < minmaxXZ && y > minYFly)
		{
			x += moveX;
			modelAngle_z = glm::min(30.f, modelAngle_z + modelAngleChange);
		}
		else if (moveX < 0 && x > -minmaxXZ && y > minYFly)
		{
			x += moveX;
			modelAngle_z = glm::max(-30.f, modelAngle_z - modelAngleChange);
		}
		else
		{
			if (modelAngle_z > 0)
				modelAngle_z -= modelAngleChange;
			if (modelAngle_z < 0)
				modelAngle_z += modelAngleChange;
		}

		if (y > minY)
		{
			motorAngle += 47;
		}
		if (motorAngle > 360)
		{
			motorAngle -= 360;
		}
	}
}

/* Called whenever the window is resized. The new window size is given, in pixels. */
static void reshape(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	aspect_ratio = ((float)w / 640.f * 4.f) / ((float)h / 480.f * 3.f);
	windowWidth = w;
	windowHeight = h;
}

/* change view angle, exit upon ESC */
static void keyCallback(GLFWwindow* window, int key, int s, int action, int mods)
{
	/* Enable this call if you want to disable key responses to a held down key*/
	//if (action != GLFW_PRESS) return;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	bool modeChanged = false;
	// sets the control mode
	if (key == '1')
	{
		controlMode = 1; // close view
		modeChanged = true;
	}
	if (key == '2')
	{
		controlMode = 2; // fly mode
		modeChanged = true;
	}
	if (key == '3')
	{
		controlMode = 3; // fly mode
		modeChanged = true;
	}
	if (key == 'G')
	{
		tree.generate("F[[-F]F[+F]]", 4);
	}
	

	if (modeChanged)
	{
		cout << "mode Changed" << endl;
		if (controlMode == 1)
		{
			angle_inc_x = 0;
			angle_inc_y = 0.05;
			angle_inc_z = 0;

			angle_x = 0;
			angle_y = 0;
			angle_z = 0;

			modelAngle_x = 0;
			modelAngle_y = 0;
			modelAngle_z = 0;

			model_scale = 1.5f;

			x = 0;
			y = -0.5;
			z = 0;

			motorAngleInc = 5;

			lightsOn = true;
		}
		else if (controlMode == 2)
		{
			angle_inc_x = 0;
			angle_inc_y = 0;
			angle_inc_z = 0;

			angle_x = 0;
			angle_y = 0;
			angle_z = 0;

			modelAngle_x = 0;
			modelAngle_y = 0;
			modelAngle_z = 0;

			model_scale = 1.f;

			x = 0;
			y = 0;
			z = 4;

			lightsOn = true;
		}
	}



	if (controlMode == 1)
	{
		if (key == 'A')//left
		{
			angle_inc_y += speed;
		}
		else if (key == 'D')//right
		{
			angle_inc_y -= speed;
		}

		if (key == 'W')//Up
		{
			if (angle_x < 1.57) // about 90deg
				angle_x += speed;

		}
		else if (key == 'S')//Down
		{
			if (angle_x > 0)
				angle_x -= speed;
		}

		if (key == 'E')// zoom in
		{
			model_scale += speed;

		}
		else if (key == 'Q')// zoom out
		{
			model_scale -= speed;
		}

		if (key == 'R')// prop speed up
		{
			motorAngleInc += 1;

		}
		else if (key == 'T')// prop speed down
		{
			motorAngleInc -= 1;
		}
	}
	else if (controlMode == 2)
	{
		if (key == 'A')//left
		{
			if (action == GLFW_PRESS)
				moveX = speed; 
			if (action == GLFW_RELEASE)
				moveX = 0;
			
		}
		else if (key == 'D')//right
		{
			if (action == GLFW_PRESS)
				moveX = -speed;
			if (action == GLFW_RELEASE)
				moveX = 0;
		}

		if (key == 'W')//forward
		{
			if (action == GLFW_PRESS)
				moveZ = speed;
			if (action == GLFW_RELEASE)
				moveZ = 0;

		}
		else if (key == 'S')//back
		{
			if (action == GLFW_PRESS)
				moveZ = -speed;
			if (action == GLFW_RELEASE)
				moveZ = 0;
		}

		if (key == 'E')//up
		{
			if (action == GLFW_PRESS)
				moveY = speed;
			if (action == GLFW_RELEASE)
				moveY = 0;

		}
		else if (key == 'Q')//down
		{
			if (action == GLFW_PRESS)
				moveY = -speed;
			if (action == GLFW_RELEASE)
				moveY = 0;
		}
	}
	else
	{
		if (key == 'Q') angle_inc_x -= speed;
		if (key == 'W') angle_inc_x += speed;
		if (key == 'E') angle_inc_y -= speed;
		if (key == 'R') angle_inc_y += speed;
		if (key == 'T') angle_inc_z -= speed;
		if (key == 'Y') angle_inc_z += speed;
		if (key == 'A') model_scale -= speed / 0.5f;
		if (key == 'S') model_scale += speed / 0.5f;

		if (key == '7') vx -= 1.f;
		if (key == '8') vx += 1.f;
		if (key == '9') vy -= 1.f;
		if (key == '0') vy += 1.f;
		if (key == 'O') vz -= 1.f;
		if (key == 'P') vz += 1.f;
	}

	/* Cycle between drawing vertices, mesh and filled polygons */
	if (key == 'F' && action == GLFW_RELEASE )
	{
		lightsOn = !lightsOn;
	}

	/* Cycle between drawing vertices, mesh and filled polygons */
	if (key == ',' && action != GLFW_RELEASE)
	{
		drawmode++;
		if (drawmode > 2) drawmode = 0;
	}

}


/* Entry point of program */
int main(int argc, char* argv[])
{
	GLWrapper* glw = new GLWrapper(1024, 768, "Assignment 1 - Drone");;
	windowWidth = 1024;
	windowHeight = 768;

	if (!ogl_LoadFunctions())
	{
		fprintf(stderr, "ogl_LoadFunctions() failed. Exiting\n");
		return 0;
	}

	glw->setRenderer(display);
	glw->setKeyCallback(keyCallback);
	glw->setReshapeCallback(reshape);

	/* Output the OpenGL vendor and version */
	glw->DisplayVersion();

	init(glw);

	glw->eventLoop();

	delete(glw);
	return 0;
}

