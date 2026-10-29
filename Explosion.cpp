#include "Explosion.h"


#define EXPLOSION_TIME 600


Explosion::Explosion()
{
	sprite = NULL;
}

Explosion::~Explosion()
{
	if(sprite != NULL)
		delete sprite;
}

void Explosion::init(const glm::ivec2 &tile, ExplosionPiece piece, int tileSize, const glm::ivec2 &tileMapPos, Texture *spritesheet, ShaderProgram &shaderProgram)
{
	tilePos = tile;
	timeLeft = EXPLOSION_TIME;

	sprite = Sprite::createSprite(glm::ivec2(tileSize, tileSize), glm::vec2(0.25f, 0.2f), spritesheet, &shaderProgram);
	sprite->setNumberAnimations(1);
	sprite->setAnimationSpeed(0, 10); // 6 keyframes en 600 ms
	float row = 0.2f * piece;
	const int frames[6] = { 0, 1, 2, 2, 1, 0 }; // crece y se apaga
	for(int i = 0; i < 6; i++)
		sprite->addKeyframe(0, glm::vec2(0.25f * frames[i], row));
	sprite->changeAnimation(0);
	sprite->setPosition(glm::vec2(float(tileMapPos.x + tile.x * tileSize), float(tileMapPos.y + tile.y * tileSize)));
}

void Explosion::update(int deltaTime)
{
	timeLeft -= deltaTime;
	sprite->update(deltaTime);
}

void Explosion::render()
{
	sprite->render();
}
