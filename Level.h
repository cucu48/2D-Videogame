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
#include "Pickup.h"
#include "Door.h"
#include "Hud.h"
#include "VsScreen.h"
#include "DitherFade.h"
#include "TextMenu.h"
#include <vector>
#include <map>


// Level contains all the entities of one stage of the game.
// It is responsible for updating and render them.


// Cascada: al darle con una explosion en la cabeza, el agua se retira hacia
// arriba tile a tile. El tile final (el que toca el suelo) sube con el agua y
// al acabar queda un unico tile de agua bajo la cabeza.
struct Waterfall
{
	glm::ivec2 head;    // tile de la cabeza (no desaparece)
	int bottomRow;      // ultima fila de agua
	int nextRow;        // fila inferior actual (la que lleva el tile final)
	int endTile;        // tile del extremo inferior, el que toca el suelo
	int timer;
	bool bRaising;
};


enum LevelState
{
	LEVEL_PLAYING,
	LEVEL_INTRO,     // boss level: "VS" screen before playing (everything stopped)
	LEVEL_CLEARED,   // the player reached the open door: "WORLD CLEAR!!" and next level
	LEVEL_GAME_OVER, // no lives left: "GAME OVER" and back to the menu
	LEVEL_SECRET_FADE // jefe derrotado con los 4 bongs y quinto bong cogido: fundido al final secreto
};


class Level : public Scene
{

public:
	Level(int number);               // number = stage (1..), loads levels/Stage1-<number>.txt
	~Level();

	void init();
	void onEnter();                  // restarts the stage
	void onResume();                 // back from the main menu ("Continuar"): as it was
	void update(int deltaTime);
	void render(int windowWidth, int windowHeight);
	void mouseMove(int x, int y);
	void mousePress(int button);
	void keyPressed(int key);        // D detonates a bomb (remote controller); debug: H toggles the heart, K kills every enemy, G god mode

	// true si la caja (en pixeles del mapa) toca algun tile en llamas.
	// Pensado tambien para los enemigos cuando se implementen.
	// Con onlyPlayerFire solo cuentan las explosiones de las bombas del jugador.
	bool isOnFire(const glm::ivec2 &pos, const glm::ivec2 &size, bool onlyPlayerFire = false) const;

private:
	void loadLevel();
	void updateCamera(bool snap = false);
	void cleanup();
	void loadEntities(const string &entitiesFile);   // enemies, boss, door, waterfalls and hidden items
	void hitWaterfall(const glm::ivec2 &tile);  // una explosion llega a este tile
	void updateWaterfalls(int deltaTime);
	bool isDoorOpen() const;         // no enemies left
	bool playerAtDoor() const;
	bool isClosedDoorTile(const glm::ivec2 &tile) const;
	void destroyBush(const glm::ivec2 &tile);   // swaps it for sky and reveals the item hidden under it, if any
	void updatePickups();
	void applyPowerUp(PowerUpType type);
	int currentBlastRange() const;   // con el Full Fire, el maximo
	std::string levelMusic() const;  // music of this stage (boss, or one per stage)
	void finishLevel();              // goes to the next level (or the menu after the last one)
	void placeBomb();
	void hurtPlayer();               // the heart absorbs the hit, otherwise the player dies
	bool touchesPlayer(const glm::ivec2 &pos, const glm::ivec2 &size) const; // and the player can be hurt
	void updateEnemies(int deltaTime, bool frozen);   // frozen: the timer stops them (dying ones still finish dying)
	void updateBoss(int deltaTime, bool frozen);
	void updateProjectiles(int deltaTime, bool frozen);
	void bossAttack(TrentAttack attack);
	void addEnemy(int type, const glm::ivec2 &pos, int dir);
	void explodeBomb(int index);
	bool addExplosion(const glm::ivec2 &tile, ExplosionPiece piece, bool fromEnemy); // false if a wall stops it
	int findBomb(const glm::ivec2 &tile) const; // index in bombs, or -1
	void updateBong(int deltaTime);
	void renderBong();
	void updateFinalBong(int deltaTime);
	// Pausa (Esc): oscurece la partida y muestra "Continuar / Opcions / Menu principal";
	// en opciones se cambia el volumen. La musica baja al 50% mientras tanto.
	void openPause();
	void closePause();
	void showPauseScreen(bool options);
	void pauseChoose(int item);
	void changeVolume(int step);
	void renderPause();
	bool pauseMousePos(glm::vec2 &p) const;
	bool overlapsTile(const glm::ivec2 &pos, const glm::ivec2 &size, const glm::ivec2 &tile) const;

private:
	int number;
	TileMap *map;
	Player *player;
	glm::vec2 spawnPos;
	int deathTime;
	bool bombKeyWasPressed;
	Texture bombSheet, explosionSheet, enemySheet, powerUpSheet, doorSheet;
	vector<Enemy *> enemies;
	vector<Bomb *> bombs;
	vector<Explosion *> explosions;
	vector<Projectile *> projectiles;
	vector<Pickup *> pickups;
	std::map<int, PowerUpType> itemUnderBush;   // key: row * map width + col, the bush that hides it
	Door *door;                     // NULL if the level has no door
	vector<Waterfall> waterfalls;
	Trent *boss;                    // NULL if the level has no boss
	float currentTime;
	int timeLeft;                   // ms
	int maxBombs;                   // bombs that can be on the map at the same time
	int blastRange;                 // tiles the explosion reaches in each direction (sin contar el Full Fire)
	bool bFullFire;                 // Full Fire: alcance maximo, solo durante este stage
	bool remoteEnabled;             // remote controller: D detonates a bomb
	int freezeTime;                 // ms left of the timer power-up: clock and enemies stopped
	Hud hud;
	VsScreen vsScreen;
	LevelState state;
	int stateTime;                  // ms since the current state started
	bool bBossLevel;
	int initialEnemies;             // without a door, the level is cleared once they are all gone
	// Coleccionable (bong) escondido en el nivel: "bong col row" en el fichero de entidades
	Texture bongTex;
	Sprite *bongSprite;
	glm::ivec2 bongTile;
	bool bBongHere;                 // esta en el mapa y aun no se ha cogido
	int bongPopupTime;              // ms que quedan del aviso "N de 4" sobre el jugador
	// Quinto bong: cae en el centro del mapa del jefe si se le derrota con los 4
	bool bFinalBong;
	glm::vec2 finalBongPos;
	float finalBongVel;
	DitherFade fade;
	bool bPaused;
	bool bPauseOptions;             // en la pantalla de opciones
	TextMenu pauseMenu;
	Texture clearTex, gameOverTex;
	Sprite *clearSprite, *gameOverSprite;
	glm::mat4 projection;
	glm::vec2 camera;               // esquina superior izquierda de la vista, en pixeles del mapa

};


#endif // _LEVEL_INCLUDE

