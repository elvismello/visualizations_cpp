#pragma once
#include <GL/glew.h>

// Shaders
GLuint makeShader(GLenum type, const char* src);
GLuint makeProgram(const char* vs, const char* fs);



