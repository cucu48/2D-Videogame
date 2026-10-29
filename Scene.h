#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE


#include <glm/glm.hpp>
#include "ShaderProgram.h"
#include "TileMap.h"
#include "Player.h"
#include "Bomb.h"
#include "Explosion.h"
#include "Enemy.h"
#include <vector>


// Scene contains all the entities of our game.
// It is responsible for updating and render them.


class Scene
{

public:
	Scene();
	~Scene();

	void init();
	void update(int deltaTime);
	void render(int windowWidth, int windowHeight);

	// true si la caja (en pixeles del mapa) toca algun tile en llamas.
	// Pensado tambien para los enemigos cuando se implementen.
	bool isOnFire(const glm::ivec2 &pos, const glm::ivec2 &size) const;

private:
	void initShaders();
	void loadEnemies(const string &enemiesFile);
	void placeBomb();
	void explodeBomb(int index);
	void addExplosion(const glm::ivec2 &tile, ExplosionPiece piece);
	bool overlapsTile(const glm::ivec2 &pos, const glm::ivec2 &size, const glm::ivec2 &tile) const;

private:
	TileMap *map;
	Player *player;
	glm::vec2 spawnPos;
	int deathTime;
	bool bombKeyWasPressed;
	Texture bombSheet, explosionSheet, enemySheet;
	vector<Enemy *> enemies;
	vector<Bomb *> bombs;
	vector<Explosion *> explosions;
	ShaderProgram texProgram;
	float currentTime;
	glm::mat4 projection;

};


#endif // _SCENE_INCLUDE

