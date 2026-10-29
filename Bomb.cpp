#include "Bomb.h"


#define BOMB_TIME 3000


Bomb::Bomb()
{
	sprite = NULL;
}

Bomb::~Bomb()
{
	if(sprite != NULL)
		delete sprite;
}

void Bomb::init(const glm::ivec2 &tile, int tileSize, const glm::ivec2 &tileMapPos, Texture *spritesheet, ShaderProgram &shaderProgram)
{
	tilePos = tile;
	timeLeft = BOMB_TIME;
	passable = true;

	sprite = Sprite::createSprite(glm::ivec2(tileSize, tileSize), glm::vec2(0.25f, 1.f), spritesheet, &shaderProgram);
	sprite->setNumberAnimations(1);
	sprite->setAnimationSpeed(0, 6);
	sprite->addKeyframe(0, glm::vec2(0.f, 0.f));
	sprite->addKeyframe(0, glm::vec2(0.25f, 0.f));
	sprite->addKeyframe(0, glm::vec2(0.5f, 0.f));
	sprite->addKeyframe(0, glm::vec2(0.75f, 0.f));
	sprite->changeAnimation(0);
	sprite->setPosition(glm::vec2(float(tileMapPos.x + tile.x * tileSize), float(tileMapPos.y + tile.y * tileSize)));
}

void Bomb::update(int deltaTime)
{
	timeLeft -= deltaTime;
	sprite->update(deltaTime);
}

void Bomb::render()
{
	sprite->render();
}
