#ifndef _TRENT_INCLUDE
#define _TRENT_INCLUDE


#include "Sprite.h"
#include "TileMap.h"



enum TrentAttack
{
	TRENT_NO_ATTACK, TRENT_LEAVES, TRENT_POLLEN, TRENT_BOMB_RAIN
};


class Trent
{
public:
	Trent();
	~Trent();

	void init(const glm::ivec2 &tileMapPos, Texture *spritesheet, ShaderProgram &shaderProgram);
	void update(int deltaTime, const glm::ivec2 &playerCenter, bool playerAlive);
	void render();

	void setTileMap(TileMap *tileMap) { map = tileMap; }
	void setPosition(const glm::ivec2 &pos);

	glm::ivec2 hitboxPos() const;
	glm::ivec2 hitboxSize() const;
	glm::ivec2 getPosition() const { return pos; }
	int spriteSize() const { return 64; }

	bool hit();
	void kill();
	TrentAttack takeAttack();
	bool isDying() const { return bDying; }
	bool isGone() const { return bDying && dyingTime <= 0; }

private:
	int step(float speed);
	void changeAnim(int anim);

private:
	glm::ivec2 tileMapDispl, pos;
	int topY;
	int direction;
	float subPixel;
	int state, stateTime;
	int attackTimer;
	TrentAttack chosenAttack, pendingAttack;
	bool bPlayerWasInFront;
	int hitsLeft;
	int invulnerableTime;
	bool bDying;
	int dyingTime;
	Sprite *sprite;
	TileMap *map;

};


#endif
