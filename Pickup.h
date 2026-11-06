#ifndef _PICKUP_INCLUDE
#define _PICKUP_INCLUDE


#include <string>
#include "Sprite.h"


enum PowerUpType
{
	PU_BOMB_UP, PU_FIRE_UP, PU_SPEED_UP, PU_REMOTE, PU_ARMOR, PU_FULL_FIRE, PU_HEART, PU_TIMER, PU_WINGS
};

bool parsePowerUp(const std::string &name, PowerUpType *type);


class Pickup
{

public:
	Pickup();
	~Pickup();

	void init(PowerUpType type, const glm::ivec2 &tile, int tileSize, Texture *spritesheet, ShaderProgram &shaderProgram);
	void render();

	PowerUpType getType() const { return type; }
	glm::ivec2 getTile() const { return tilePos; }
	glm::ivec2 hitboxPos() const { return tilePos * tileSize; }
	glm::ivec2 hitboxSize() const { return glm::ivec2(tileSize, tileSize); }

private:
	PowerUpType type;
	glm::ivec2 tilePos;
	int tileSize;
	Sprite *sprite;

};


#endif
