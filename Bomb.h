#ifndef _BOMB_INCLUDE
#define _BOMB_INCLUDE


#include "Sprite.h"



class Bomb
{
public:
	Bomb();
	~Bomb();

	void init(const glm::ivec2 &tile, int tileSize, const glm::ivec2 &tileMapPos, Texture *spritesheet, ShaderProgram &shaderProgram);
	void update(int deltaTime);
	void render();

	glm::ivec2 getTile() const { return tilePos; }
	bool hasToExplode() const { return timeLeft <= 0; }
	void detonate(int skipArmDir) { if(timeLeft > 0) { timeLeft = 0; skipArm = skipArmDir; } }
	void detonate() { timeLeft = 0; }
	int getSkipArm() const { return skipArm; }

	bool isPassable() const { return passable; }
	void makeSolid() { passable = false; }

	void setEnemyBomb(int range, bool horizontalOnly, int fuseTime);
	bool isEnemyBomb() const { return enemyBomb; }
	int getRange() const { return range; }
	bool isHorizontalOnly() const { return horizontalOnly; }

private:
	glm::ivec2 tilePos;
	int timeLeft;
	bool passable;
	bool enemyBomb, horizontalOnly;
	int range;
	int skipArm;
	Sprite *sprite;

};


#endif
