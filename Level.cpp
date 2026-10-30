#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "Level.h"
#include "Game.h"
#include <algorithm>
#include <fstream>
#include <sstream>



#define INIT_PLAYER_X_TILES 4
#define MAX_BOMBS 1
#define RESPAWN_TIME 2000


Level::Level(int number)
{
	this->number = number;
	map = NULL;
	player = NULL;
}

Level::~Level()
{
	cleanup();
}

// Frees everything created by loadLevel
void Level::cleanup()
{
	if(map != NULL)
		delete map;
	map = NULL;
	if(player != NULL)
		delete player;
	player = NULL;
	for(size_t i = 0; i < bombs.size(); i++)
		delete bombs[i];
	bombs.clear();
	for(size_t i = 0; i < explosions.size(); i++)
		delete explosions[i];
	explosions.clear();
	for(size_t i = 0; i < enemies.size(); i++)
		delete enemies[i];
	enemies.clear();
}

void Level::init()
{
	bombSheet.loadFromFile("images/bomb.png", TEXTURE_PIXEL_FORMAT_RGBA);
	bombSheet.setMinFilter(GL_NEAREST);
	bombSheet.setMagFilter(GL_NEAREST);
	explosionSheet.loadFromFile("images/explosion.png", TEXTURE_PIXEL_FORMAT_RGBA);
	explosionSheet.setMinFilter(GL_NEAREST);
	explosionSheet.setMagFilter(GL_NEAREST);
	enemySheet.loadFromFile("images/enemies.png", TEXTURE_PIXEL_FORMAT_RGBA);
	enemySheet.setMinFilter(GL_NEAREST);
	enemySheet.setMagFilter(GL_NEAREST);
	loadLevel();
}

// Every time the level is entered it starts from scratch
void Level::onEnter()
{
	cleanup();
	loadLevel();
}

void Level::loadLevel()
{
	ShaderProgram &texProgram = Game::instance().getTexProgram();
	string stage = "levels/Stage1-" + to_string(number);
	map = TileMap::createTileMap(stage + ".txt", glm::vec2(0, 0), texProgram);
	player = new Player();
	player->init(glm::ivec2(0, 0), texProgram);
	spawnPos = glm::vec2(map->findSpawn(INIT_PLAYER_X_TILES, glm::ivec2(32, 32)));
	player->setPosition(spawnPos);
	player->setTileMap(map);
	loadEnemies(stage + "_enemies.txt");
	deathTime = 0;
	bombKeyWasPressed = false;
	glm::vec2 mapPx = glm::vec2(map->getMapSize()) * float(map->getTileSize());
	projection = glm::ortho(0.f, mapPx.x, mapPx.y, 0.f);
	currentTime = 0.0f;
}

void Level::update(int deltaTime)
{
	currentTime += deltaTime;
	player->update(deltaTime);

	// Z: place bomb
	bool bombKey = Game::instance().getKey(GLFW_KEY_Z);
	if(bombKey && !bombKeyWasPressed && !player->isDead())
		placeBomb();
	bombKeyWasPressed = bombKey;

	// Bombas: cuenta atras y paso de "atravesable" a "solida"
	for(size_t i = 0; i < bombs.size(); i++)
	{
		bombs[i]->update(deltaTime);
		if(bombs[i]->isPassable() && !overlapsTile(player->hitboxPos(), player->hitboxSize(), bombs[i]->getTile()))
		{
			bombs[i]->makeSolid();
			map->setBlocked(bombs[i]->getTile().x, bombs[i]->getTile().y, true);
		}
	}
	for(size_t i = 0; i < bombs.size(); )
	{
		if(bombs[i]->hasToExplode())
			explodeBomb(i);
		else
			i++;
	}

	// Explosions
	for(size_t i = 0; i < explosions.size(); )
	{
		explosions[i]->update(deltaTime);
		if(explosions[i]->isFinished())
		{
			delete explosions[i];
			explosions.erase(explosions.begin() + i);
		}
		else
			i++;
	}

	// Enemigos: se mueven, mueren con el fuego y matan al jugador si le tocan
	for(size_t i = 0; i < enemies.size(); )
	{
		Enemy *enemy = enemies[i];
		enemy->update(deltaTime);
		if(!enemy->isDying() && isOnFire(enemy->hitboxPos(), enemy->hitboxSize()))
			enemy->kill();
		if(!enemy->isDying() && !player->isDead() && !player->isInvulnerable())
		{
			glm::ivec2 pp = player->hitboxPos(), ps = player->hitboxSize();
			glm::ivec2 ep = enemy->hitboxPos(), es = enemy->hitboxSize();
			if(pp.x < ep.x + es.x && pp.x + ps.x > ep.x && pp.y < ep.y + es.y && pp.y + ps.y > ep.y)
			{
				player->kill();
				deathTime = 0;
			}
		}
		if(enemy->isGone())
		{
			delete enemy;
			enemies.erase(enemies.begin() + i);
		}
		else
			i++;
	}

	// Damage to player
	if(!player->isDead() && !player->isInvulnerable() && isOnFire(player->hitboxPos(), player->hitboxSize()))
	{
		player->kill();
		deathTime = 0;
	}
	if(player->isDead())
	{
		deathTime += deltaTime;
		if(deathTime >= RESPAWN_TIME)
			player->respawn(spawnPos);
	}
}

// Lee los enemigos del nivel. Formato (una linea por enemigo):
//   tipo columna fila direccion
// tipo = fila de images/enemies.png (0-6), direccion = -1 izquierda / 1 derecha.
// Las lineas que empiezan por # son comentarios.
void Level::loadEnemies(const string &enemiesFile)
{
	ifstream fin(enemiesFile.c_str());
	if(!fin.is_open())
		return;
	string line;
	while(getline(fin, line))
	{
		if(line.empty() || line[0] == '#')
			continue;
		stringstream ss(line);
		int type, col, row, dir;
		if(!(ss >> type >> col >> row >> dir) || type < 0 || type > 6)
			continue;
		Enemy *enemy = new Enemy();
		enemy->init(type, glm::ivec2(0, 0), &enemySheet, Game::instance().getTexProgram());
		enemy->setTileMap(map);
		enemy->setPosition(glm::ivec2(col, row) * map->getTileSize());
		enemy->setDirection(dir);
		enemies.push_back(enemy);
	}
}

void Level::placeBomb()
{
	if(int(bombs.size()) >= MAX_BOMBS)
		return;

	// tile donde esta el centro del jugador
	int tileSize = map->getTileSize();
	glm::ivec2 center = player->hitboxPos() + player->hitboxSize() / 2;
	glm::ivec2 tile(center.x / tileSize, center.y / tileSize);
	if(map->isSolid(tile.x, tile.y))
		return; // ya hay una bomba (o una pared) en ese tile

	Bomb *bomb = new Bomb();
	bomb->init(tile, tileSize, glm::ivec2(0, 0), &bombSheet, Game::instance().getTexProgram());
	bombs.push_back(bomb);
}

void Level::explodeBomb(int index)
{
	glm::ivec2 tile = bombs[index]->getTile();
	map->setBlocked(tile.x, tile.y, false);
	delete bombs[index];
	bombs.erase(bombs.begin() + index);

	addExplosion(tile, EXPLOSION_CENTER);
	addExplosion(tile + glm::ivec2(0, -1), EXPLOSION_UP);
	addExplosion(tile + glm::ivec2(0, 1), EXPLOSION_DOWN);
	addExplosion(tile + glm::ivec2(-1, 0), EXPLOSION_LEFT);
	addExplosion(tile + glm::ivec2(1, 0), EXPLOSION_RIGHT);
}

void Level::addExplosion(const glm::ivec2 &tile, ExplosionPiece piece)
{
	if(piece != EXPLOSION_CENTER && map->isWall(tile.x, tile.y))
		return;

	Explosion *explosion = new Explosion();
	explosion->init(tile, piece, map->getTileSize(), glm::ivec2(0, 0), &explosionSheet, Game::instance().getTexProgram());
	explosions.push_back(explosion);

}

bool Level::isOnFire(const glm::ivec2 &pos, const glm::ivec2 &size) const
{
	for(size_t i = 0; i < explosions.size(); i++)
		if(overlapsTile(pos, size, explosions[i]->getTile()))
			return true;
	return false;
}

bool Level::overlapsTile(const glm::ivec2 &pos, const glm::ivec2 &size, const glm::ivec2 &tile) const
{
	int tileSize = map->getTileSize();
	int tx = tile.x * tileSize, ty = tile.y * tileSize;
	return pos.x < tx + tileSize && pos.x + size.x > tx &&
	       pos.y < ty + tileSize && pos.y + size.y > ty;
}

void Level::render(int windowWidth, int windowHeight)
{
	glm::vec2 mapPx = glm::vec2(map->getMapSize()) * float(map->getTileSize());
	projection = setLetterboxViewport(windowWidth, windowHeight, int(mapPx.x), int(mapPx.y));

	glm::mat4 modelview;

	ShaderProgram &texProgram = Game::instance().getTexProgram();
	texProgram.use();
	texProgram.setUniformMatrix4f("projection", projection);
	texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
	modelview = glm::mat4(1.0f);
	texProgram.setUniformMatrix4f("modelview", modelview);
	texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);
	map->render();
	for(size_t i = 0; i < bombs.size(); i++)
		bombs[i]->render();
	for(size_t i = 0; i < enemies.size(); i++)
		enemies[i]->render();
	for(size_t i = 0; i < explosions.size(); i++)
		explosions[i]->render();
	player->render();
}

