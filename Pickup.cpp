#include "Pickup.h"


// Cell (row, col) of each power-up in images/PowerUps.png
struct PowerUpInfo
{
	const char *name;
	int row, col;
};

static const PowerUpInfo POWER_UPS[] = {
	{ "bombup",   0, 0 },   // PU_BOMB_UP
	{ "fireup",   0, 1 },   // PU_FIRE_UP
	{ "speedup",  0, 2 },   // PU_SPEED_UP
	{ "remote",   3, 0 },   // PU_REMOTE
	{ "armor",    2, 0 },   // PU_ARMOR
	{ "fullfire", 1, 2 },   // PU_FULL_FIRE
	{ "heart",    2, 1 },   // PU_HEART
	{ "timer",    1, 1 },   // PU_TIMER
	{ "wings",    2, 2 }    // PU_WINGS
};
static const int NUM_POWER_UPS = sizeof(POWER_UPS) / sizeof(POWER_UPS[0]);


bool parsePowerUp(const std::string &name, PowerUpType *type)
{
	for(int i = 0; i < NUM_POWER_UPS; i++)
		if(name == POWER_UPS[i].name)
		{
			*type = PowerUpType(i);
			return true;
		}
	return false;
}


Pickup::Pickup()
{
	sprite = NULL;
}

Pickup::~Pickup()
{
	if(sprite != NULL)
		delete sprite;
}

void Pickup::init(PowerUpType type, const glm::ivec2 &tile, int tileSize, Texture *spritesheet, ShaderProgram &shaderProgram)
{
	this->type = type;
	this->tilePos = tile;
	this->tileSize = tileSize;

	sprite = Sprite::createSprite(glm::ivec2(tileSize, tileSize), glm::vec2(0.25f, 0.25f), spritesheet, &shaderProgram);
	sprite->setNumberAnimations(1);
	sprite->setAnimationSpeed(0, 1);
	sprite->addKeyframe(0, glm::vec2(0.25f * POWER_UPS[type].col, 0.25f * POWER_UPS[type].row));
	sprite->changeAnimation(0);
	sprite->setPosition(glm::vec2(float(tile.x * tileSize), float(tile.y * tileSize)));
}

void Pickup::render()
{
	sprite->render();
}
