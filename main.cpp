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

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pocket Bomberman", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Set window initial position */
	glfwSetWindowPos(window, 100, 100);
	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	/* Sync with the monitor so the loop doesn't burn the CPU */
	glfwSwapInterval(1);

	/* Set callbacks */
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	/* Init glew to have access to GL extensions */
	glewExperimental = GL_TRUE;
	glewInit();

	/* Init step of the game loop */
	Game::instance().init();
	int fbWidth, fbHeight;
	glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
	Game::instance().resize(fbWidth, fbHeight);
	timePreviousFrame = glfwGetTime();

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		currentTime = glfwGetTime();
		accumulator += currentTime - timePreviousFrame;
		timePreviousFrame = currentTime;
		if (accumulator > 0.25)	// avoid a burst of updates after a long pause
			accumulator = 0.25;

		/* Game logic always advances in fixed 1/60 s steps */
		while (accumulator >= timePerFrame)
		{
			if(!Game::instance().update(int(1000.0f * timePerFrame)))
				glfwSetWindowShouldClose(window, GLFW_TRUE);
			accumulator -= timePerFrame;
		}

		Game::instance().render();
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

