// NOTE:  THERE IS NOTHING COMPUTE SHADER SPECIFIC IN THIS FILE
#include "my_opengl.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>

GLuint genRenderProg(GLuint /*texHandle*/)
{
	GLuint progHandle = glCreateProgram();
	GLuint vp = glCreateShader(GL_VERTEX_SHADER);
	GLuint fp = glCreateShader(GL_FRAGMENT_SHADER);

	const char* vpSrc[] =
	{
		"#version 430\n",
		"in vec2 pos;"
		"out vec2 texCoord;"
		"void main() {"
		"	texCoord = pos * 0.5f + 0.5f;"
		"	gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);"
		"}"
	};

	const char* fpSrc[] =
	{
		"#version 430\n",
		"uniform sampler2D srcTex;"
		"in vec2 texCoord;"
		"out vec4 color;"
		"void main() {"
		"	float c = texture(srcTex, texCoord).x;"
		"	color = vec4(c, 1.0, 1.0, 1.0);"
		"}"
	};

	glShaderSource(vp, 2, vpSrc, nullptr);
	glShaderSource(fp, 2, fpSrc, nullptr);

	glCompileShader(vp);
	int rvalue = 0;
	glGetShaderiv(vp, GL_COMPILE_STATUS, &rvalue);
	if (!rvalue)
	{
		fprintf(stderr, "Error in compiling vp\n");
		exit(30);
	}
	glAttachShader(progHandle, vp);
	glDeleteShader(vp);
	vp = 0;

	glCompileShader(fp);
	glGetShaderiv(fp, GL_COMPILE_STATUS, &rvalue);
	if (!rvalue)
	{
		fprintf(stderr, "Error in compiling fp\n");
		exit(31);
	}
	glAttachShader(progHandle, fp);
	glDeleteShader(fp);
	fp = 0;

	glBindFragDataLocation(progHandle, 0, "color");
	glLinkProgram(progHandle);

	glGetProgramiv(progHandle, GL_LINK_STATUS, &rvalue);
	if (!rvalue)
	{
		fprintf(stderr, "Error in linking sp\n");
		exit(32);
	}   

	glUseProgram(progHandle);
	glUniform1i(glGetUniformLocation(progHandle, "srcTex"), 0);

	GLuint vertArray = 0;
	glGenVertexArrays(1, &vertArray);
	glBindVertexArray(vertArray);

	GLuint posBuf = 0;
	glGenBuffers(1, &posBuf);
	glBindBuffer(GL_ARRAY_BUFFER, posBuf);
	const float data[] =
	{
		-1.0f, -1.0f,
		-1.0f, +1.0f,
		+1.0f, -1.0f,
		+1.0f, +1.0f,
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STREAM_DRAW);

	GLint posPtr = glGetAttribLocation(progHandle, "pos");
	glVertexAttribPointer(posPtr, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(posPtr);

	checkErrors("Render shaders");
	return progHandle;
}

GLuint genTexture()
{
	// We create a single float channel 512^2 texture
	GLuint texHandle = 0;
	glGenTextures(1, &texHandle);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texHandle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, TEX_WIDTH, TEX_HEIGHT, 0, GL_RED, GL_FLOAT, nullptr);

	// Because we're also using this tex as an image (in order to write to it),
	// we bind it to an image unit as well
	glBindImageTexture(0, texHandle, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	checkErrors("Gen texture");
	return texHandle;
}

void checkErrors(const char* desc)
{
	const GLenum e = glGetError();
	if (e != GL_NO_ERROR)
	{
		fprintf(stderr, "OpenGL error in \"%s\": %s (%d)\n", desc, gluErrorString(e), e);
		exit(20);
	}
}

void initGL()
{
	printf(
		"OpenGL:\n"
		"	Vendor: %s\n"
		"	Renderer: %s\n"
		"	Version: %s\n"
		"	Shader Language: %s\n",
		glGetString(GL_VENDOR),
		glGetString(GL_RENDERER),
		glGetString(GL_VERSION),
		glGetString(GL_SHADING_LANGUAGE_VERSION));

	// Finding the compute shader extension
	if (GLEW_ARB_compute_shader)
	{
		printf("Extension \"GL_ARB_compute_shader\" found\n");
	}
	else
	{
		fprintf(stderr, "Extension \"GL_ARB_compute_shader\" not found\n");
		exit(14);
	}

	glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);

	checkErrors("GL init");
}
