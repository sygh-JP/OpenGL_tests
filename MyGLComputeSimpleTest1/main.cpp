// Original Code:
// http://wili.cc/blog/opengl-cs.html

// For Microsoft Visual C++:
// Append global include and library directory paths to GLEW and GLFW.


#if !defined(__cplusplus)
#error C++ compiler required.
#endif

#if (__cplusplus < 201103L)
#pragma message("Your compiler does not fully-support C++11.")
#endif


#include "my_opengl.h"


namespace
{
	GLuint g_renderHandle, g_computeHandle;
	int g_frame;

	void updateTex(int frame)
	{
		glUseProgram(g_computeHandle);
		glUniform1f(glGetUniformLocation(g_computeHandle, "roll"), (frame * 0.01f));
		glDispatchCompute(TEX_WIDTH / 16, TEX_HEIGHT / 16, 1); // 512^2 threads in blocks of 16^2
		checkErrors("Dispatch compute shader");
	}

	void drawScreen()
	{
		glUseProgram(g_renderHandle);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		checkErrors("Draw screen");
	}

	void advanceFrame()
	{
		++g_frame;
		if (g_frame >= 1024)
		{
			g_frame = 0;
		}
	}

	void onResize(int w, int h)
	{
		//glutReshapeWindow(WIN_WIDTH, WIN_HEIGHT);
	}
}

int main(int argc, char* argv[])
{
	if (glfwInit() == GL_FALSE)
	{
		printf("Failed to initialize GLFW!!\n");
		return -1;
	}

	// OpenGL Version 4.3 Core Profile を選択する。
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	auto window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "OpenGL Compute Shader Simple Test", nullptr, nullptr);
	if (window == nullptr)
	{
		printf("Failed to create GLFW window!!\n");
		return -1;
	}

	// GLUT/FreeGLUT と違い、GLFW はクライアント領域のサイズを明示的に設定できる。
	glfwSetWindowSize(window, WIN_WIDTH, WIN_HEIGHT);

	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK)
	{
		printf("Failed to initialize GLEW!!\n");
		return -1;
	}

	// 垂直同期のタイミングを待つ。
	glfwSwapInterval(1);

	initGL();

	GLuint texHandle = genTexture();
	g_renderHandle = genRenderProg(texHandle);
	g_computeHandle = genComputeProg(texHandle);

	while (glfwWindowShouldClose(window) == GL_FALSE)
	{
		updateTex(g_frame);
		drawScreen();
		advanceFrame();

		glfwSwapBuffers(window);
		checkErrors("Swapping bufs");
		//glfwWaitEvents();
		glfwPollEvents();
	}

	// TODO: GL リソースの破棄。

	glfwTerminate();

	return 0;
}
