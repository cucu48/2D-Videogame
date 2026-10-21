#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Game.h"


void Game::init()
{
	bPlay = true;
	windowWidth = SCREEN_WIDTH;
	windowHeight = SCREEN_HEIGHT;
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	scene.init();
}

bool Game::update(int deltaTime)
{
	scene.update(deltaTime);

	return bPlay;
}

void Game::render()
{
	glViewport(0, 0, windowWidth, windowHeight); // toda la ventana
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // pinta las bandas de negro
	scene.render(windowWidth, windowHeight);
}

void Game::resize(int width, int height)
{
	windowWidth = width;
	windowHeight = height;
}

void Game::keyPressed(int key)
{
	if(key < 0 || key > GLFW_KEY_LAST)
		return;
	if(key == GLFW_KEY_ESCAPE)
		bPlay = false;
	keys[key] = true;
}

void Game::keyReleased(int key)
{
	if(key < 0 || key > GLFW_KEY_LAST)
		return;
	keys[key] = false;
}

void Game::mouseMove(int x, int y)
{
}

void Game::mousePress(int button)
{
}

void Game::mouseRelease(int button)
{
}

bool Game::getKey(int key) const
{
	return keys[key];
}



