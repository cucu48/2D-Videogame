#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Game.h"
#include "MenuScene.h"
#include "LevelSelectScene.h"
#include "Level.h"
#include "SecretScene.h"
#include "InstructionsScene.h"
#include "CreditsScene.h"
#include "Audio.h"
#include "Sounds.h"


#define TRANSITION_TIME 200
#define TRANSITION_W 320
#define TRANSITION_H 240


void Game::init()
{
	bPlay = true;
	bGodMode = false;
	windowWidth = SCREEN_WIDTH;
	windowHeight = SCREEN_HEIGHT;
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	initShaders();

	scenes.push_back(new MenuScene());
	scenes.push_back(new LevelSelectScene());
	for(int i = 1; i <= NUM_LEVELS; i++)
		scenes.push_back(new Level(i));
	scenes.push_back(new SecretScene());
	scenes.push_back(new InstructionsScene());
	scenes.push_back(new CreditsScene());
	for(size_t i = 0; i < scenes.size(); i++)
		scenes[i]->init();

	for(size_t i = 0; i < sizeof(ALL_SFX) / sizeof(ALL_SFX[0]); i++)
		Audio::instance().preload(ALL_SFX[i]);

	transition.init(texProgram, TRANSITION_W, TRANSITION_H);
	doChangeScene(SCENE_MENU);
}

Game::~Game()
{
	for(size_t i = 0; i < scenes.size(); i++)
		delete scenes[i];
	texProgram.free();
}

void Game::changeScene(int index, bool immediate)
{
	if(index < 0 || index >= int(scenes.size()))
		return;
	if(immediate || index == SCENE_SECRET)
	{
		pendingScene = NO_PENDING;
		bFadingOut = false;
		fadeTime = TRANSITION_TIME;
		doChangeScene(index);
		return;
	}
	if(pendingScene != NO_PENDING)
		return;
	pendingScene = index;
	fadeTime = (!bFadingOut && fadeTime < TRANSITION_TIME) ? TRANSITION_TIME - fadeTime : 0;
	bFadingOut = true;
}

void Game::doChangeScene(int index)
{
	currentScene = index;
	if(isLevelScene(index))
		suspendedScene = -1;
	if(!isLevelScene(index) && index != SCENE_SECRET)
	{
		lives = START_LIVES;
		powerUpState = PowerUpState();
	}
	scenes[currentScene]->onEnter();
}

void Game::suspendToMenu()
{
	suspendedScene = currentScene;
	suspendedLives = lives;
	suspendedPowerUps = powerUpState;
	changeScene(SCENE_MENU);
}

void Game::resumeSuspendedGame()
{
	if(suspendedScene < 0 || pendingScene != NO_PENDING)
		return;
	pendingScene = PENDING_RESUME;
	fadeTime = (!bFadingOut && fadeTime < TRANSITION_TIME) ? TRANSITION_TIME - fadeTime : 0;
	bFadingOut = true;
}

void Game::doResume()
{
	if(suspendedScene < 0)
		return;
	currentScene = suspendedScene;
	suspendedScene = -1;
	lives = suspendedLives;
	powerUpState = suspendedPowerUps;
	scenes[currentScene]->onResume();
}

void Game::startLevel(int levelNumber)
{
	if(levelNumber < 1 || levelNumber > NUM_LEVELS)
		return;
	changeScene(SCENE_FIRST_LEVEL + levelNumber - 1);
}

bool Game::update(int deltaTime)
{
	if(bFadingOut)
	{
		fadeTime += deltaTime;
		if(fadeTime >= TRANSITION_TIME)
		{
			int target = pendingScene;
			pendingScene = NO_PENDING;
			bFadingOut = false;
			fadeTime = 0;
			if(target == PENDING_RESUME)
				doResume();
			else
				doChangeScene(target);
		}
		return bPlay;
	}
	if(fadeTime < TRANSITION_TIME)
		fadeTime += deltaTime;
	scenes[currentScene]->update(deltaTime);

	return bPlay;
}

void Game::render()
{
	glViewport(0, 0, windowWidth, windowHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	scenes[currentScene]->render(windowWidth, windowHeight);

	int level = 0;
	if(bFadingOut)
		level = fadeTime * (FADE_LEVELS + 1) / TRANSITION_TIME;
	else if(fadeTime < TRANSITION_TIME)
		level = FADE_LEVELS - fadeTime * (FADE_LEVELS + 1) / TRANSITION_TIME;
	if(level > 0)
	{
		glViewport(0, 0, windowWidth, windowHeight);
		transition.render(texProgram, level);
	}
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
	keys[key] = true;
	if(bFadingOut)
		return;

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

	if(key == GLFW_KEY_E)
	{
		for(int i = 0; i < NUM_BONGS; i++)
			bongs[i] = true;
		Audio::instance().playSound(SFX_COLLECT);
		std::cout << "Truco: todos los bongs" << std::endl;
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
	if(bFadingOut)
		return;
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
