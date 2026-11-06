#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Game.h"


#define TARGET_FRAMERATE 60.0f


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
		Game::instance().keyPressed(key);
	else if (action == GLFW_RELEASE)
		Game::instance().keyReleased(key);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	Game::instance().resize(width, height);
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	int winWidth, winHeight, fbWidth, fbHeight;
	glfwGetWindowSize(window, &winWidth, &winHeight);
	glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
	if(winWidth > 0 && winHeight > 0)
	{
		xpos *= double(fbWidth) / winWidth;
		ypos *= double(fbHeight) / winHeight;
	}
	Game::instance().mouseMove(int(xpos), int(ypos));
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS)
		Game::instance().mousePress(button);
	else if (action == GLFW_RELEASE)
		Game::instance().mouseRelease(button);
}


int main(void)
{
	GLFWwindow* window;
	double timePerFrame = 1.f / TARGET_FRAMERATE, timePreviousFrame, currentTime;
	double accumulator = 0.0;

	if (!glfwInit())
		return -1;

	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pocket Bomberman", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwSetWindowPos(window, 100, 100);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glewExperimental = GL_TRUE;
	glewInit();

	Game::instance().init();
	int fbWidth, fbHeight;
	glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
	Game::instance().resize(fbWidth, fbHeight);
	timePreviousFrame = glfwGetTime();

	while (!glfwWindowShouldClose(window))
	{
		currentTime = glfwGetTime();
		accumulator += currentTime - timePreviousFrame;
		timePreviousFrame = currentTime;
		if (accumulator > 0.25)
			accumulator = 0.25;

		while (accumulator >= timePerFrame)
		{
			if(!Game::instance().update(int(1000.0f * timePerFrame)))
				glfwSetWindowShouldClose(window, GLFW_TRUE);
			accumulator -= timePerFrame;
		}

		Game::instance().render();
		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

