#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>

#define WIN_WIDTH 512
#define WIN_HEIGHT 512

#define TEX_WIDTH 512
#define TEX_HEIGHT 512

extern void initGL();

// Return handles
extern GLuint genTexture();
extern GLuint genRenderProg(GLuint /*texHandle*/); // Texture as the param
extern GLuint genComputeProg(GLuint /*texHandle*/);

extern void checkErrors(const char* desc);
