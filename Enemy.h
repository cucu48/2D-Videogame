#ifndef _ENEMY_INCLUDE
#define _ENEMY_INCLUDE


#include "Sprite.h"
#include "TileMap.h"



enum EnemyType
{
	JACKY, EYTI, GREEN, ORANGE, BLACK, CLOUDY, SPRINGCHICKEN, ELEKIN, LEAF, NUM_ENEMY_TYPES
};


class Enemy
{
public:
	Enemy();
	~Enemy();

	void init(int type, const glm::ivec2 &tileMapPos, Texture *spritesheet, ShaderProgram &shaderProgram);
	void update(int deltaTime, const glm::ivec2 &playerCenter, bool playerAlive);
	void render();

	void setTileMap(TileMap *tileMap) { map = tileMap; }
	void setPosition(const glm::ivec2 &pos);
	void setDirection(int dir);
	glm::ivec2 spriteSize() const { return size; }

	glm::ivec2 hitboxPos() const;
	glm::ivec2 hitboxSize() const;

	bool attackBox(glm::ivec2 &pos, glm::ivec2 &boxSize) const;
	bool takeShot(glm::vec2 &pos, glm::vec2 &dir);

	void kill();
	bool isDying() const { return bDying; }
	bool isGone() const { return bDying && dyingTime <= 0; }

private:
	void updateJacky(int deltaTime);
	void updateEyti(int deltaTime, const glm::ivec2 &playerCenter, bool playerAlive);
	void updateJumper(int deltaTime);
	void updateCloudy(int deltaTime, const glm::ivec2 &playerCenter, bool playerAlive);
	void updateSpringchicken(int deltaTime);
	void updateElekin(int deltaTime, const glm::ivec2 &playerCenter, bool playerAlive);
	void updateLeaf(int deltaTime, const glm::ivec2 &playerCenter);

	bool fall();
	int step(float speed);
	bool moveX(int dx);
	bool groundAhead() const;
	bool seesPlayer(const glm::ivec2 &playerCenter) const;
	void turnAround();
	void updateAnimation();

private:
	int type;
	glm::ivec2 tileMapDispl, posEnemy, size;
	int direction;
	float velY;
	float subPixel;
	bool bOnGround;
	int state, stateTime, timer;
	int groundY;
	int walked;
	float flyTime;
	int flyBaseY;
	int shotCooldown;
	bool bShot;
	glm::vec2 shotDir;
	glm::vec2 leafPos;
	int lifeTime;
	bool bDying;
	int dyingTime;
	Sprite *sprite, *boltSprite;
	TileMap *map;

};


#endif
