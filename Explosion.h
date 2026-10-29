#ifndef _EXPLOSION_INCLUDE
#define _EXPLOSION_INCLUDE


#include "Sprite.h"



enum ExplosionPiece
{
	EXPLOSION_CENTER, EXPLOSION_UP, EXPLOSION_DOWN, EXPLOSION_LEFT, EXPLOSION_RIGHT
};


class Explosion
{
public:
	Explosion();
	~Explosion();

	void init(const glm::ivec2 &tile, ExplosionPiece piece, int tileSize, const glm::ivec2 &tileMapPos, Texture *spritesheet, ShaderProgram &shaderProgram);
	void update(int deltaTime);
	void render();

	glm::ivec2 getTile() const { return tilePos; }
	bool isFinished() const { return timeLeft <= 0; }

private:
	glm::ivec2 tilePos;
	int timeLeft;
	Sprite *sprite;

};


#endif // _EXPLOSION_INCLUDE
