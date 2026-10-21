#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene.h"
#include "Game.h"
#include <algorithm>



#define INIT_PLAYER_X_TILES 4


Scene::Scene()
{
	map = NULL;
	player = NULL;
}

Scene::~Scene()
{
	texProgram.free();
	if(map != NULL)
		delete map;
	if(player != NULL)
		delete player;
}


void Scene::init()
{
	initShaders();
	map = TileMap::createTileMap("levels/Stage1-1.txt", glm::vec2(0, 0), texProgram);
	player = new Player();
	player->init(glm::ivec2(0, 0), texProgram);
	player->setPosition(glm::vec2(map->findSpawn(INIT_PLAYER_X_TILES, glm::ivec2(32, 32))));
	player->setTileMap(map);
	glm::vec2 mapPx = glm::vec2(map->getMapSize()) * float(map->getTileSize());
	projection = glm::ortho(0.f, mapPx.x, mapPx.y, 0.f);
	currentTime = 0.0f;
	//glClearColor(0.f, 0.f, 0.f, 1.f);
}

void Scene::update(int deltaTime)
{
	currentTime += deltaTime;
	player->update(deltaTime);
}

void Scene::render(int windowWidth, int windowHeight)
{
	glm::vec2 mapPx = glm::vec2(map->getMapSize()) * float(map->getTileSize());
	float scale = std::min(windowWidth / mapPx.x, windowHeight / mapPx.y);
	int w = int(mapPx.x * scale), h = int(mapPx.y * scale);
	glViewport((windowWidth - w) / 2, (windowHeight - h) / 2, w, h);

	glm::mat4 modelview;

	texProgram.use();
	texProgram.setUniformMatrix4f("projection", projection);
	texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
	modelview = glm::mat4(1.0f);
	texProgram.setUniformMatrix4f("modelview", modelview);
	texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);
	map->render();
	player->render();
}

void Scene::initShaders()
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



