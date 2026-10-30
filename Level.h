#ifndef _LEVEL_INCLUDE
#define _LEVEL_INCLUDE


#include <glm/glm.hpp>
#include "ShaderProgram.h"
#include "Scene.h"
#include "TileMap.h"
#include "Player.h"
#include "Bomb.h"
#include "Explosion.h"
#include "Enemy.h"
#include <vector>


// Level contains all the entities of one stage of the game.
// It is responsible for updating and render them.


class Level : public Scene
{

public:
	Level(int number);               // number = stage (1..), loads levels/Stage1-<number>.txt
	~Level();

	void init();
	void onEnter();                  // restarts the stage
	void update(int deltaTime);
	void render(int windowWidth, int windowHeight);

	// true si la caja (en pixeles del mapa) toca algun tile en llamas.
	// Pensado tambien para los enemigos cuando se implementen.
	bool isOnFire(const glm::ivec2 &pos, const glm::ivec2 &size) const;

private:
	void loadLevel();
	void updateCamera();
	void cleanup();
	void loadEnemies(const string &enemiesFile);
	void placeBomb();
	void explodeBomb(int index);
	void addExplosion(const glm::ivec2 &tile, ExplosionPiece piece);
	bool overlapsTile(const glm::ivec2 &pos, const glm::ivec2 &size, const glm::ivec2 &tile) const;

private:
	int number;
	TileMap *map;
	Player *player;
	glm::vec2 spawnPos;
	int deathTime;
	bool bombKeyWasPressed;
	Texture bombSheet, explosionSheet, enemySheet;
	vector<Enemy *> enemies;
	vector<Bomb *> bombs;
	vector<Explosion *> explosions;
	float currentTime;
	glm::mat4 projection;
	glm::vec2 camera;               // esquina superior izquierda de la vista, en pixeles del mapa

};


#endif // _LEVEL_INCLUDE

