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
#include "Instructions.h"
#include <vector>
#include <map>



struct Waterfall
{
	glm::ivec2 head;
	int bottomRow;
	int nextRow;
	int endTile;
	int timer;
	bool bRaising;
};


enum LevelState
{
	LEVEL_PLAYING,
	LEVEL_INTRO,
	LEVEL_STAGE_CARD,
	LEVEL_CLEARED,
	LEVEL_GAME_OVER,
	LEVEL_SECRET_FADE
};


class Level : public Scene
{

public:
	Level(int number);
	~Level();

	void init();
	void onEnter();
	void onResume();
	void update(int deltaTime);
	void render(int windowWidth, int windowHeight);
	void mouseMove(int x, int y);
	void mousePress(int button);
	void keyPressed(int key);

	bool isOnFire(const glm::ivec2 &pos, const glm::ivec2 &size, bool onlyPlayerFire = false) const;

private:
	void loadLevel();
	void updateCamera(bool snap = false);
	void cleanup();
	void loadEntities(const string &entitiesFile);
	void hitWaterfall(const glm::ivec2 &tile);
	void updateWaterfalls(int deltaTime);
	bool isDoorOpen() const;
	bool playerAtDoor() const;
	bool isClosedDoorTile(const glm::ivec2 &tile) const;
	void destroyBush(const glm::ivec2 &tile);
	void updatePickups();
	void applyPowerUp(PowerUpType type);
	int currentBlastRange() const;
	std::string levelMusic() const;
	void finishLevel();
	void placeBomb();
	void hurtPlayer();
	bool touchesPlayer(const glm::ivec2 &pos, const glm::ivec2 &size) const;
	void updateEnemies(int deltaTime, bool frozen);
	void updateBoss(int deltaTime, bool frozen);
	void updateProjectiles(int deltaTime, bool frozen);
	void bossAttack(TrentAttack attack);
	void addEnemy(int type, const glm::ivec2 &pos, int dir);
	void explodeBomb(int index);
	bool addExplosion(const glm::ivec2 &tile, ExplosionPiece piece, bool fromEnemy);
	int findBomb(const glm::ivec2 &tile) const;
	void updateBong(int deltaTime);
	void renderBong();
	void updateFinalBong(int deltaTime);
	enum PauseScreen { PAUSE_MAIN, PAUSE_OPTIONS, PAUSE_HELP };
	void openPause();
	void closePause();
	void showPauseScreen(PauseScreen screen);
	void pauseChoose(int item);
	void changeVolume(int step);
	void renderPause();
	HudState hudState() const;
	void renderStageCard();
	void spawnLeafBits(const glm::ivec2 &tile);
	void updateLeafBits(int deltaTime);
	void renderLeafBits();
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
	std::map<int, PowerUpType> itemUnderBush;
	Door *door;
	vector<Waterfall> waterfalls;
	Trent *boss;
	float currentTime;
	int timeLeft;
	int maxBombs;
	int blastRange;
	bool bFullFire;
	bool remoteEnabled;
	int freezeTime;
	Hud hud;
	VsScreen vsScreen;
	LevelState state;
	int stateTime;
	bool bBossLevel;
	int initialEnemies;
	Texture bongTex;
	Sprite *bongSprite;
	glm::ivec2 bongTile;
	bool bBongHere;
	int bongPopupTime;
	bool bFinalBong;
	glm::vec2 finalBongPos;
	float finalBongVel;
	DitherFade fade;
	bool bPaused;
	PauseScreen pauseScreen;
	TextMenu pauseMenu;
	Instructions instructions;
	int powerUpFlash;
	struct LeafBit { glm::vec2 pos, vel; int life, frame; };
	vector<LeafBit> leafBits;
	Texture leafTex;
	TexturedQuad *leafQuads[4];
	Texture clearTex, gameOverTex;
	Sprite *clearSprite, *gameOverSprite;
	glm::mat4 projection;
	glm::vec2 camera;

};


#endif

