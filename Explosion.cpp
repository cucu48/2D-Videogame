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

	// explosion2.png: 8 columns x 4 rows of 16x16 cells, 3 growth frames per piece
	//   tips:   cols 0-2 -> row 0 up, row 1 down, row 2 left, row 3 right
	//   middle: cols 3-5 -> row 0 vertical, row 2 horizontal
	//   center: col 7, rows 0-2 (one growth frame per row)
	const float cellW = 0.125f, cellH = 0.25f;
	int firstCol = 0, row = 0;
	bool centerPiece = false;
	switch(piece)
	{
	case EXPLOSION_CENTER:     centerPiece = true; firstCol = 7; break;
	case EXPLOSION_UP:         firstCol = 0; row = 0; break;
	case EXPLOSION_DOWN:       firstCol = 0; row = 1; break;
	case EXPLOSION_LEFT:       firstCol = 0; row = 2; break;
	case EXPLOSION_RIGHT:      firstCol = 0; row = 3; break;
	case EXPLOSION_VERTICAL:   firstCol = 3; row = 0; break;
	case EXPLOSION_HORIZONTAL: firstCol = 3; row = 2; break;
	}

	sprite = Sprite::createSprite(glm::ivec2(tileSize, tileSize), glm::vec2(cellW, cellH), spritesheet, &shaderProgram);
	sprite->setNumberAnimations(1);
	sprite->setAnimationSpeed(0, 10); // 6 keyframes en 600 ms
	const int frames[6] = { 0, 1, 2, 2, 1, 0 }; // crece y se apaga
	for(int i = 0; i < 6; i++)
	{
		if(centerPiece)
			sprite->addKeyframe(0, glm::vec2(cellW * firstCol, cellH * frames[i]));
		else
			sprite->addKeyframe(0, glm::vec2(cellW * (firstCol + frames[i]), cellH * row));
	}
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
