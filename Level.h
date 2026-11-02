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
#include "Hud.h"
#include <vector>


// Level contains all the entities of one stage of the game.
// It is responsible for updating and render them.


enum LevelState
{
	LEVEL_PLAYING,
	LEVEL_CLEARED,   // the player reached the open door: "WORLD CLEAR!!" and next level
	LEVEL_GAME_OVER  // no lives left: "GAME OVER" and back to the menu
};


class Level : public Scene
{

public:
	Level(int number);               // number = stage (1..), loads levels/Stage1-<number>.txt
	~Level();

	void init();
	void onEnter();                  // restarts the stage
	void update(int deltaTime);
	void render(int windowWidth, int windowHeight);
	void keyPressed(int key);        // H toggles the heart power-up (debug until pickups exist)

	// true si la caja (en pixeles del mapa) toca algun tile en llamas.
	// Pensado tambien para los enemigos cuando se implementen.
	bool isOnFire(const glm::ivec2 &pos, const glm::ivec2 &size) const;

private:
	void loadLevel();
	void updateCamera();
	void cleanup();
	void loadEnemies(const string &enemiesFile);
	bool isDoorOpen() const;
	bool playerAtDoor() const;
	void finishLevel();              // goes to the next level (or the menu after the last one)
	void placeBomb();
	void hurtPlayer();               // the heart absorbs the hit, otherwise the player dies
	void explodeBomb(int index);
	int findBomb(const glm::ivec2 &tile) const; // index in bombs, or -1
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
	int timeLeft;                   // ms
	int maxBombs;                   // bombs that can be on the map at the same time
	int blastRange;                 // tiles the explosion reaches in each direction
	Hud hud;
	LevelState state;
	int stateTime;                  // ms since the current state started
	bool bHasDoor;
	int initialEnemies;             // without a door, the level is cleared once they are all gone
	glm::ivec2 doorTile;
	Texture clearTex, gameOverTex;
	Sprite *clearSprite, *gameOverSprite;
	glm::mat4 projection;
	glm::vec2 camera;               // esquina superior izquierda de la vista, en pixeles del mapa

};


#endif // _LEVEL_INCLUDE

