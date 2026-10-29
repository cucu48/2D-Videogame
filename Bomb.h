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
	void detonate() { timeLeft = 0; }

	bool isPassable() const { return passable; }
	void makeSolid() { passable = false; }

private:
	glm::ivec2 tilePos;
	int timeLeft;
	bool passable;
	Sprite *sprite;

};


#endif // _BOMB_INCLUDE
