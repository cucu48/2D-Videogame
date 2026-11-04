#ifndef _PICKUP_INCLUDE
#define _PICKUP_INCLUDE


#include <string>
#include "Sprite.h"


// Power-ups: a bush may hide one. It appears on the tile of the destroyed bush
// and is applied (by Level) when the player touches it.
enum PowerUpType
{
	PU_BOMB_UP, PU_FIRE_UP, PU_SPEED_UP, PU_REMOTE, PU_ARMOR, PU_FULL_FIRE, PU_HEART, PU_TIMER, PU_WINGS
};

// Names used in the entities file: bombup fireup speedup remote armor fullfire heart timer wings
bool parsePowerUp(const std::string &name, PowerUpType *type);


class Pickup
{

public:
	Pickup();
	~Pickup();

	// spritesheet = images/PowerUps.png (4x4 cells of 16x16)
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


#endif // _PICKUP_INCLUDE
