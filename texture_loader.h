#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include <string>

#include "wrapper_glfw.h"

#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

// loads a texture and returns the opengl index
GLuint loadTexture(std::string filename);

#endif