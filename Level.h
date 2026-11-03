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
#include "Trent.h"
#include "Projectile.h"
#include "Hud.h"
#include <vector>


// Level contains all the entities of one stage of the game.
// It is responsible for updating and render them.


// Cascada: al darle con una explosion en la cabeza, el agua se retira hacia
// arriba tile a tile y deja el paso libre.
struct Waterfall
{
	glm::ivec2 head;    // tile de la cabeza (no desaparece)
	int bottomRow;      // ultima fila de agua
	int nextRow;        // siguiente fila que se retira
	int timer;
	bool bRaising;
};


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
	void keyPressed(int key);        // H toggles the heart power-up (debug until pickups exist), K kills every enemy, G god mode

	// true si la caja (en pixeles del mapa) toca algun tile en llamas.
	// Pensado tambien para los enemigos cuando se implementen.
	// Con onlyPlayerFire solo cuentan las explosiones de las bombas del jugador.
	bool isOnFire(const glm::ivec2 &pos, const glm::ivec2 &size, bool onlyPlayerFire = false) const;

private:
	void loadLevel();
	void updateCamera();
	void cleanup();
	void loadEnemies(const string &enemiesFile);
	void hitWaterfall(const glm::ivec2 &tile);  // una explosion llega a este tile
	void updateWaterfalls(int deltaTime);
	bool isDoorOpen() const;
	bool playerAtDoor() const;
	void finishLevel();              // goes to the next level (or the menu after the last one)
	void placeBomb();
	void hurtPlayer();               // the heart absorbs the hit, otherwise the player dies
	bool touchesPlayer(const glm::ivec2 &pos, const glm::ivec2 &size) const; // and the player can be hurt
	void updateEnemies(int deltaTime);
	void updateBoss(int deltaTime);
	void updateProjectiles(int deltaTime);
	void bossAttack(TrentAttack attack);
	void addEnemy(int type, const glm::ivec2 &pos, int dir);
	void explodeBomb(int index);
	bool addExplosion(const glm::ivec2 &tile, ExplosionPiece piece, bool fromEnemy); // false if a wall stops it
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
	vector<Projectile *> projectiles;
	vector<Waterfall> waterfalls;
	Trent *boss;                    // NULL if the level has no boss
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

