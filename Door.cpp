#include "Door.h"


enum DoorAnims
{
	DOOR_CLOSED, DOOR_OPEN
};


Door::Door()
{
	sprite = NULL;
	map = NULL;
}

Door::~Door()
{
	if(sprite != NULL)
		delete sprite;
}

void Door::init(const glm::ivec2 &tile, TileMap *tileMap, Texture *spritesheet, ShaderProgram &shaderProgram)
{
	tilePos = tile;
	map = tileMap;
	bOpen = false;
	int tileSize = map->getTileSize();

	sprite = Sprite::createSprite(glm::ivec2(tileSize, tileSize), glm::vec2(0.5f, 1.f), spritesheet, &shaderProgram);
	sprite->setNumberAnimations(2);
	sprite->setAnimationSpeed(DOOR_CLOSED, 1);
	sprite->addKeyframe(DOOR_CLOSED, glm::vec2(0.f, 0.f));
	sprite->setAnimationSpeed(DOOR_OPEN, 1);
	sprite->addKeyframe(DOOR_OPEN, glm::vec2(0.5f, 0.f));
	sprite->changeAnimation(DOOR_CLOSED);
	sprite->setPosition(glm::vec2(float(tile.x * tileSize), float(tile.y * tileSize)));

	map->setBlocked(tile.x, tile.y, true);
}

void Door::open()
{
	if(bOpen)
		return;
	bOpen = true;
	sprite->changeAnimation(DOOR_OPEN);
	map->setBlocked(tilePos.x, tilePos.y, false);
}

void Door::render()
{
	sprite->render();
}
