#include "my_opengl.h"
#include <cstdio>
#include <cstdlib>

GLuint genComputeProg(GLuint /*texHandle*/)
{
	// Creating the compute shader, and the program object containing the shader
	GLuint progHandle = glCreateProgram();
	GLuint cs = glCreateShader(GL_COMPUTE_SHADER);

	// In order to write to a texture, we have to introduce it as image2D.
	// local_size_x/y/z layout variables define the work group size.
	// gl_GlobalInvocationID is a uvec3 variable giving the global ID of the thread,
	// gl_LocalInvocationID is the local index within the work group, and
	// gl_WorkGroupID is the work group's index
	const char* csSrc[] =
	{
		"#version 430\n",
		"uniform float roll;"
		"uniform writeonly image2D destTex;" // "error C7596: OpenGL requires image variables declared without using the 'writeonly' qualifier to have a format layout qualifier"
		"layout (local_size_x = 16, local_size_y = 16) in;"
		"void main() {"
		"	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);"
		"	float localCoef = length(vec2(ivec2(gl_LocalInvocationID.xy) - 8) / 8.0);"
		"	float globalCoef = sin(float(gl_WorkGroupID.x + gl_WorkGroupID.y) * 0.1 + roll) * 0.5;"
		"	imageStore(destTex, storePos, vec4(1.0 - globalCoef * localCoef, 0.0, 0.0, 0.0));" // Unordered access
		"}"
	};

	glShaderSource(cs, 2, csSrc, nullptr);
	glCompileShader(cs);
	int rvalue = 0;
	glGetShaderiv(cs, GL_COMPILE_STATUS, &rvalue);
	if (!rvalue)
	{
		fprintf(stderr, "Error in compiling the compute shader\n");
		GLsizei logLength = 1;
		glGetShaderiv(cs, GL_INFO_LOG_LENGTH, &logLength); // Last null will be included.
		std::vector<GLchar> log(logLength);
		glGetShaderInfoLog(cs, logLength - 1, &logLength, &log[0]);
		fprintf(stderr, "Compiler log:\n%s\n", &log[0]);
		exit(40);
	}
	glAttachShader(progHandle, cs);

	glLinkProgram(progHandle);
	glGetProgramiv(progHandle, GL_LINK_STATUS, &rvalue);
	if (!rvalue)
	{
		fprintf(stderr, "Error in linking compute shader program\n");
		GLsizei logLength = 1;
		glGetProgramiv(progHandle, GL_INFO_LOG_LENGTH, &logLength); // Last null will be included.
		std::vector<GLchar> log(logLength);
		glGetProgramInfoLog(progHandle, logLength - 1, &logLength, &log[0]);
		fprintf(stderr, "Linker log:\n%s\n", &log[0]);
		exit(41);
	}
	glUseProgram(progHandle);

	glUniform1i(glGetUniformLocation(progHandle, "destTex"), 0);

	checkErrors("Compute shader");
	return progHandle;
}
