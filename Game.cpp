#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Game.h"
#include "MenuScene.h"
#include "LevelSelectScene.h"
#include "Level.h"


void Game::init()
{
	bPlay = true;
	windowWidth = SCREEN_WIDTH;
	windowHeight = SCREEN_HEIGHT;
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	initShaders();

	scenes.push_back(new MenuScene());
	scenes.push_back(new LevelSelectScene());
	for(int i = 1; i <= NUM_LEVELS; i++)
		scenes.push_back(new Level(i));
	for(size_t i = 0; i < scenes.size(); i++)
		scenes[i]->init();

	changeScene(SCENE_MENU);
}

Game::~Game()
{
	for(size_t i = 0; i < scenes.size(); i++)
		delete scenes[i];
	texProgram.free();
}

void Game::changeScene(int index)
{
	if(index < 0 || index >= int(scenes.size()))
		return;
	currentScene = index;
	scenes[currentScene]->onEnter();
}

void Game::startLevel(int levelNumber)
{
	if(levelNumber < 1 || levelNumber > NUM_LEVELS)
		return;
	changeScene(SCENE_FIRST_LEVEL + levelNumber - 1);
}

bool Game::update(int deltaTime)
{
	scenes[currentScene]->update(deltaTime);

	return bPlay;
}

void Game::render()
{
	glViewport(0, 0, windowWidth, windowHeight); // toda la ventana
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // pinta las bandas de negro
	scenes[currentScene]->render(windowWidth, windowHeight);
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

	// Number keys go straight to the matching level from any scene
	int levelNumber = 0;
	if(key >= GLFW_KEY_1 && key <= GLFW_KEY_9)
		levelNumber = key - GLFW_KEY_1 + 1;
	else if(key >= GLFW_KEY_KP_1 && key <= GLFW_KEY_KP_9)
		levelNumber = key - GLFW_KEY_KP_1 + 1;
	if(levelNumber >= 1 && levelNumber <= NUM_LEVELS)
	{
		startLevel(levelNumber);
		return;
	}

	scenes[currentScene]->keyPressed(key);
}

void Game::keyReleased(int key)
{
	if(key < 0 || key > GLFW_KEY_LAST)
		return;
	keys[key] = false;
}

void Game::mouseMove(int x, int y)
{
	mouseX = x;
	mouseY = y;
	scenes[currentScene]->mouseMove(x, y);
}

void Game::mousePress(int button)
{
	scenes[currentScene]->mousePress(button);
}

void Game::mouseRelease(int button)
{
	scenes[currentScene]->mouseRelease(button);
}

bool Game::getKey(int key) const
{
	return keys[key];
}




void Game::initShaders()
{
	Shader vShader, fShader;

	vShader.initFromFile(VERTEX_SHADER, "shaders/texture.vert");
	if(!vShader.isCompiled())
	{
		cout << "Vertex Shader Error" << endl;
		cout << "" << vShader.log() << endl << endl;
	}
	fShader.initFromFile(FRAGMENT_SHADER, "shaders/texture.frag");
	if(!fShader.isCompiled())
	{
		cout << "Fragment Shader Error" << endl;
		cout << "" << fShader.log() << endl << endl;
	}
	texProgram.init();
	texProgram.addShader(vShader);
	texProgram.addShader(fShader);
	texProgram.link();
	if(!texProgram.isLinked())
	{
		cout << "Shader Linking Error" << endl;
		cout << "" << texProgram.log() << endl << endl;
	}
	texProgram.bindFragmentOutput("outColor");
	vShader.free();
	fShader.free();
}
