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
	// Chain reaction: explode now. skipArm is the arm (0 up, 1 down, 2 left, 3 right) that
	// points back at the bomb that struck this one, and it does not blast. Ignored if already detonating.
	void detonate(int skipArmDir) { if(timeLeft > 0) { timeLeft = 0; skipArm = skipArmDir; } }
	void detonate() { timeLeft = 0; }
	int getSkipArm() const { return skipArm; }

	bool isPassable() const { return passable; }
	void makeSolid() { passable = false; }

private:
	glm::ivec2 tilePos;
	int timeLeft;
	bool passable;
	int skipArm;
	Sprite *sprite;

};


#endif // _BOMB_INCLUDE
