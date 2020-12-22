
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
#include "wrapper_glfwv2.h"
#include <iostream>
#include <stack>
#include <chrono>

   /* Include GLM core and matrix extensions*/
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

// Include headers for our objects
#include "tree.h"
#include "terrain_object.h"

#include "drone.h"

#include "directional_light.h"

#include "texture_loader.h"

#include "lights_uniform_block_wrapper.h"

#include "common_includes.h"

/* Define buffer object indices */
GLuint elementbuffer;

GLuint programs[NUM_PROGRAMS];		/* Identifier for the shader prgoram */
GLuint vao;			/* Vertex array (Containor) object. This is the index of the VAO that will be the container for
					   our buffer objects */

// globals for shadow mapping
//GLuint shadowProgram;	// shader program for shadow rendering
GLuint depthMapFBO; // depth map for shadows

GLuint depthMap; // idx for texture for shadow map
GLuint dirDepthMapArray; // idx for texture array for directional shadow map

const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

/* Position and view globals */
GLuint drawmode;			// Defines drawing mode of sphere as points, lines or filled polygons
GLfloat speed;				// movement increment
GLfloat motorAngle;				
GLfloat motorAngleInc;				
bool lightsOn;
bool useCascadingShadows;

// globals for animation
GLfloat modelAngle_x, modelAngle_y, modelAngle_z, modelAngleChange;
GLfloat moveX, moveY, moveZ;


const int numTerrainTextures = 4;


/* Uniforms*/
GLuint modelID[NUM_PROGRAMS], viewID[NUM_PROGRAMS], projectionID[NUM_PROGRAMS], normalMatrixID[NUM_PROGRAMS], viewPosID[NUM_PROGRAMS];
GLuint colourModeID[NUM_PROGRAMS], emitModeID[NUM_PROGRAMS];
GLuint colourOverrideID[NUM_PROGRAMS], reflectivenessID[NUM_PROGRAMS];
GLuint shadowMapID[NUM_PROGRAMS], lightSpaceMatrixID[NUM_PROGRAMS];
GLuint textureID[NUM_PROGRAMS], terrainTextureID[numTerrainTextures], terrainTextureThresholdID[numTerrainTextures], useTextureID[NUM_PROGRAMS];
GLuint roughnessID[NUM_PROGRAMS], terrainRoughnessID[numTerrainTextures], useRoughnessID[NUM_PROGRAMS];
GLuint lightParamsID[NUM_PROGRAMS];
GLuint dirShadowMapArrayID[NUM_PROGRAMS];
GLuint farPlanesID[NUM_PROGRAMS][NUM_FAR_PLANES];

LightsUniformWrapper lightsUniformBlock;

DirectionalLight directionalLight;

GLfloat aspect_ratio;		/* Aspect ratio of the window defined in the reshape callback*/
int windowWidth, windowHeight;
GLuint numspherevertices;


/* Global instances of our objects */
Tree tree;
terrain_object* terrain;
Drone drone;

std::vector<vec3> treeLocations;


// texture IDs
GLuint texture[totalNumTextures];
GLuint normalMap[totalNumTextures];
GLuint roughnessMap[totalNumTextures];

using namespace std;
using namespace glm;

/*
This function is called before entering the main rendering loop.
Use it for all your initialisation stuff
*/
void init(GLWrapperV2* glw)
{
	/* Set the object transformation controls to their initial values */
	speed = 0.025f;
	aspect_ratio = 1.3333f;
	motorAngle = 0;

	//control mode 2 defaults
	modelAngleChange = 2.f;
	lightsOn = true;
	useCascadingShadows = false;

	lightsUniformBlock.resetLights();


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


	/* Load and build the vertex and fragment shaders */
	try
	{
		programs[OMNI_SHADOW_PROGRAM] = glw->LoadShader("shaders\\omnishadows.vert", "shaders\\omnishadows.geom", "shaders\\omnishadows.frag");
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
	glGenTextures(1, &dirDepthMapArray);
	glBindTexture(GL_TEXTURE_2D_ARRAY, dirDepthMapArray);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, MAX_LIGHTS, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.f, 1.f, 1.f, 1.f };
	glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);
	glBindTexture(GL_TEXTURE_2D, 0);


	// attaches the texture to the frame buffer
	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, dirDepthMapArray, 0, 0);
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
		glUseProgram(programs[i]);
		modelID[i] = glGetUniformLocation(programs[i], "model");

		if (i != SHADOW_PROGRAM)
		{
			colourModeID[i] = glGetUniformLocation(programs[i], "colourMode");
			emitModeID[i] = glGetUniformLocation(programs[i], "emitMode");
			viewID[i] = glGetUniformLocation(programs[i], "view");
			projectionID[i] = glGetUniformLocation(programs[i], "projection");
			normalMatrixID[i] = glGetUniformLocation(programs[i], "normalMatrix");

			lightParamsID[i] = glGetUniformBlockIndex(programs[i], "LightParams");
			glUniformBlockBinding(programs[i], lightParamsID[i], LIGHT_PARAMS_BINDING);


			viewPosID[i] = glGetUniformLocation(programs[i], "viewPos");
			colourOverrideID[i] = glGetUniformLocation(programs[i], "colourOverride");
			reflectivenessID[i] = glGetUniformLocation(programs[i], "reflectiveness");

			shadowMapID[i] = glGetUniformLocation(programs[i], "shadowMap");
			dirShadowMapArrayID[i] = glGetUniformLocation(programs[i], "shadowMapArr");

			useTextureID[i] = glGetUniformLocation(programs[i], "useTex");
			useRoughnessID[i] = glGetUniformLocation(programs[i], "useRoughness");

			for (int j = 0; j < NUM_FAR_PLANES; j++)
			{
				std::string str = "farPlanes[" + std::to_string(j) + "]";
				farPlanesID[i][j] = glGetUniformLocation(programs[i], str.c_str());
				glUniform1f(farPlanesID[i][j], std::pow(((float)(j + 1) / NUM_FAR_PLANES), 2.f)* (FAR_PLANE_DIST - 2.f) + 2.f);
			}
			if (i == MAIN_PROGRAM)
			{
				textureID[i] = glGetUniformLocation(programs[i], "tex");
				roughnessID[i] = glGetUniformLocation(programs[i], "roughness");
			}
			else if (i == TERRAIN_PROGRAM)
			{
				for (int j = 0; j < numTerrainTextures; j++)
				{
					std::string str = "tex[" + std::to_string(j) + "]";
					terrainTextureID[j] = glGetUniformLocation(programs[i], str.c_str());
					str = "texThres[" + std::to_string(j) + "]";
					terrainTextureThresholdID[j] = glGetUniformLocation(programs[i], str.c_str());
					str = "roughness[" + std::to_string(j) + "]";
					terrainRoughnessID[j] = glGetUniformLocation(programs[i], str.c_str());
				}
			}
		}
		else
		{
			lightSpaceMatrixID[i] = glGetUniformLocation(programs[i], "lightSpaceMatrix");
		}
		glUseProgram(0);
	}


	tree.generate("F[[-F]F[+F]]", 3);

	directionalLight.dir = vec3(1.f, -2.5f, 2.f);

	terrain = new terrain_object(4, 1.f, 2.f);
	terrain->createTerrain(100, 100, 20.f, 20.f);
	terrain->setColourBasedOnHeight();
	terrain->createObject();


	for (int i = 0; i < 10; i++)
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
			roughnessMap[i] = loadTexture("textures/Bark_004_roughness.jpg");
			break;
		}
		case(SAND):
		{
			texture[i] = loadTexture("textures/Sand_007_basecolor.jpg");
			normalMap[i] = loadTexture("textures/Sand_007_normal.jpg");
			roughnessMap[i] = loadTexture("textures/Sand_007_roughness.jpg");
			break;
		}
		case(GRASS):
		{
			texture[i] = loadTexture("textures/Stylized_Grass_001_basecolor.jpg");
			normalMap[i] = loadTexture("textures/Stylized_Grass_001_normal.jpg");
			roughnessMap[i] = loadTexture("textures/Stylized_Grass_001_roughness.jpg");
			break;
		}
		case(DIRT):
		{
			texture[i] = loadTexture("textures/Ground_wet_003_basecolor.jpg");
			normalMap[i] = loadTexture("textures/Ground_wet_003_normal.jpg");
			roughnessMap[i] = loadTexture("textures/Ground_wet_003_roughness.jpg");
			break;
		}
		case(ROCK):
		{
			texture[i] = loadTexture("textures/Rock_028_COLOR.jpg");
			normalMap[i] = loadTexture("textures/Rock_028_NORM.jpg");
			roughnessMap[i] = loadTexture("textures/Rock_028_ROUGH.jpg");
			break;
		}
		case(CARBON):
		{
			texture[i] = loadTexture("textures/Carbon_Fiber_001_basecolor.jpg");
			normalMap[i] = loadTexture("textures/Carbon_Fiber_001_normal.jpg");
			roughnessMap[i] = loadTexture("textures/Carbon_Fiber_001_roughness.jpg");
			break;
		}
		}
	}

	DroneUniforms droneuniforms = { textureID, roughnessID, useTextureID, useRoughnessID,
		modelID, normalMatrixID, colourOverrideID, reflectivenessID };

	drone.init(droneuniforms, texture[CARBON], roughnessMap[CARBON], vec3(0.f, terrain->heightAtPosition(0.f,0.f) + 0.2f, 0.f));
	drone.droneScale = 0.35f;
	

	// print instructions
	cout << endl <<
		"####\\/ Drone Simulator \\/####" << endl << 
		endl <<
		"##### Fly Mode #####" << endl <<
		"[Q]: Fly down/ land" << endl <<
		"[E]: Fly up/ takeoff" << endl <<
		"[W]: Fly forward" << endl <<
		"[S]: Fly backward" << endl <<
		"[A]: Turn left" << endl <<
		"[D]: Turn right" << endl <<
		"[F] Turn lights on the drone on/off (on by default)" << endl << 
		endl << 
		"##### Other Settings #####" << endl <<
		"[,] Switch between draw modes to see the triangles or vertices" << endl;

}

void render(mat4& view, GLuint programID)
{

	drone.draw(drawmode, programID, view);

	mat3 normalmatrix;
	stack<mat4> model;
	model.push(mat4(1.0f));

	// trees
	model.push(model.top());
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture[BARK]);
		glUniform1i(textureID[programID], 0);

		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, roughnessMap[BARK]);
		glUniform1i(roughnessID[programID], 0);


		glUniform1i(useTextureID[programID], 1);
		glUniform1i(useRoughnessID[programID], 1);

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
		glUniform1i(useRoughnessID[programID], 0);
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

	for (int i = numTerrainTextures; i < numTerrainTextures*2; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		int textureIdx = SAND + i - numTerrainTextures;

		glBindTexture(GL_TEXTURE_2D, roughnessMap[textureIdx]);
		glUniform1i(terrainRoughnessID[textureIdx], i);
	}


	glUniform1i(useTextureID[programID], 1);
	glUniform1i(useRoughnessID[programID], 1);


	// set the reflectiveness uniform
	glUniform1f(reflectivenessID[programID], 0.f);
	// set the colour uniform
	glUniform1ui(colourModeID[programID], 0);
	// Send the model uniform and normal matrix to the currently bound shader,
	glUniformMatrix4fv(modelID[programID], 1, GL_FALSE, &(model[0][0]));
	// Recalculate the normal matrix and send to the vertex shader
	mat3 normalmatrix = transpose(inverse(mat3(view *model)));
	glUniformMatrix3fv(normalMatrixID[programID], 1, GL_FALSE, &normalmatrix[0][0]);

	terrain->drawObject(drawmode);

	glUniform1i(useRoughnessID[programID], 0);
	glUniform1i(useTextureID[programID], 0);
}

void resetLights()
{
	lightsUniformBlock.resetLights();
}


/* Called to update the display. Note that this function is called in the event loop in the wrapper
   class because we registered display as a callback function */
void display()
{
	// Define the normal matrix
	mat3 normalmatrix;

	// Projection matrix
	mat4 renderProjection;

	// Camera matrix
	mat4 renderView;


	renderProjection = perspective(radians(60.f), aspect_ratio, 0.1f, FAR_PLANE_DIST);

	GLfloat temp = drone.pos.x / drone.pos.z;
	if (abs(drone.pos.x) < 0.01 || abs(drone.pos.z) < 0.01)
		temp = 0;

	vec3 cameraPos = mat3(rotate(mat4(1.f), -radians(drone.orient.y), vec3(0.f, 1.f, 0.f))) * vec3(0.f, (1.2f * drone.droneScale), -(3.f * drone.droneScale)) + drone.pos;

	renderView = lookAt(
		cameraPos,
		vec3(drone.pos.x, drone.pos.y, drone.pos.z),
		vec3(0, 1, 0)  
	);




	// render shadow maps

	resetLights();

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glUseProgram(programs[SHADOW_PROGRAM]);

	if (useCascadingShadows)
	{

		float currFarPlane, currNearPlane = 0.1f;
		mat4 shadowView = renderView;

		directionalLight.cascading = true;
		directionalLight.lightSpace.resize(NUM_FAR_PLANES);


		for (int i = 0; i < NUM_FAR_PLANES; i++)
		{
			glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, dirDepthMapArray, 0, i);
			glClear(GL_DEPTH_BUFFER_BIT);
			

			currFarPlane = std::pow(((float)(i + 1) / NUM_FAR_PLANES), 2.f) * (FAR_PLANE_DIST - 2.f) + 2.f;
			mat4 cascadingRenderProjection = perspective(radians(60.f), aspect_ratio, currNearPlane, currFarPlane);

			mat4 lightSpace = directionalLight.genLightProjView(shadowView, cascadingRenderProjection, i);
			glUniformMatrix4fv(lightSpaceMatrixID[SHADOW_PROGRAM], 1, GL_FALSE, &lightSpace[0][0]);
			

			glEnable(GL_DEPTH_CLAMP);
			render(renderView, SHADOW_PROGRAM);
			glDisable(GL_DEPTH_CLAMP);

			shadowView = translate(shadowView, vec3(0.f, 0.f, currFarPlane - currNearPlane));
			currNearPlane = currFarPlane;

		}
	}
	else
	{
		directionalLight.lightSpace.resize(1);
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, dirDepthMapArray, 0, 0);
		glClear(GL_DEPTH_BUFFER_BIT);


		directionalLight.cascading = false;
		mat4 lightSpace = directionalLight.genLightProjView(renderView, renderProjection);
		glUniformMatrix4fv(lightSpaceMatrixID[SHADOW_PROGRAM], 1, GL_FALSE, &lightSpace[0][0]);
		

		glEnable(GL_DEPTH_CLAMP);
		render(renderView, SHADOW_PROGRAM);
		renderTerrain(renderView, SHADOW_PROGRAM);
		glDisable(GL_DEPTH_CLAMP);
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);




	
	// render camera view
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


	lightsUniformBlock.resetLights();
	directionalLight.setUniforms(lightsUniformBlock);
	if (lightsOn)
		drone.setLightUniforms(lightsUniformBlock, renderView);

	lightsUniformBlock.bind(LIGHT_PARAMS_BINDING);

	glUniformMatrix4fv(viewID[MAIN_PROGRAM], 1, GL_FALSE, &renderView[0][0]);
	glUniformMatrix4fv(projectionID[MAIN_PROGRAM], 1, GL_FALSE, &renderProjection[0][0]);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D_ARRAY, dirDepthMapArray);
	glUniform1i(dirShadowMapArrayID[MAIN_PROGRAM], 5);


	render(renderView,MAIN_PROGRAM);

	glUseProgram(programs[TERRAIN_PROGRAM]);


	glUniformMatrix4fv(viewID[TERRAIN_PROGRAM], 1, GL_FALSE, &renderView[0][0]);
	glUniformMatrix4fv(projectionID[TERRAIN_PROGRAM], 1, GL_FALSE, &renderProjection[0][0]);

	// binds depth map array
	glActiveTexture(GL_TEXTURE0 + numTerrainTextures*2);
	glBindTexture(GL_TEXTURE_2D_ARRAY, dirDepthMapArray);
	glUniform1i(dirShadowMapArrayID[TERRAIN_PROGRAM], numTerrainTextures * 2);

	renderTerrain(renderView,TERRAIN_PROGRAM);

	glDisableVertexAttribArray(0);
	glUseProgram(0);

	/* Modify our animation variables */

	GLfloat minmaxXZ = 9.5f;
	GLfloat maxY = 15.f;
	GLfloat minY = terrain->heightAtPosition(drone.pos.x,drone.pos.z) + 0.2f;

	vec3 pos = vec3(0), orient = vec3(0);


	if (moveY > 0 && drone.pos.y < maxY)
	{
		pos.y += moveY;
	}
	else if (moveY < 0 && drone.pos.y > minY)
	{
		pos.y += moveY;
	}

	if (moveZ > 0 && drone.pos.z < minmaxXZ && drone.pos.y > minY)
	{
		pos.z += moveZ;
		orient.x = - modelAngleChange;
	}
	else if (moveZ < 0 && drone.pos.z > -minmaxXZ && drone.pos.y > minY)
	{
		pos.z = moveZ;
		orient.x = modelAngleChange;
	}
	else
	{
		if (drone.orient.x > 0)
			orient.x -= modelAngleChange;
		if (drone.orient.x < 0)
			orient.x += modelAngleChange;
	}


	if (moveX > 0 && drone.pos.x < minmaxXZ && drone.pos.y > minY)
	{
		//drone.pos.x += moveX;
		orient.z = modelAngleChange;
		orient.y = -0.5 * modelAngleChange;
	}
	else if (moveX < 0 && drone.pos.x > -minmaxXZ && drone.pos.y > minY)
	{
		//drone.pos.x += moveX;
		orient.z = -modelAngleChange;
		orient.y = 0.5 * modelAngleChange;
	}
	else
	{
		if (drone.orient.z > 0)
			orient.z -= modelAngleChange;
		if (drone.orient.z < 0)
			orient.z += modelAngleChange;
	}

	drone.move(pos, orient);

	if (drone.pos.y > minY)
	{
		drone.spinMotor();
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


static void keyCallback(GLFWwindow* window, int key, int s, int action, int mods)
{

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == 'G')
	{
		tree.generate("F[[-F]F[+F]]", 4);
	}

	if (key == 'H' && action == GLFW_PRESS)
	{
		useCascadingShadows = !useCascadingShadows;
	}


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

	// turn drone lights on/ off
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
	GLWrapperV2* glw = new GLWrapperV2(1024, 768, "Assignment 2 - Drone");;
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

