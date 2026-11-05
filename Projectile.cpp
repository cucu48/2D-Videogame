#include <cmath>
#include <algorithm>
#include "Projectile.h"


#define SHEET_W 256.f
#define SHEET_H 512.f
#define PROJECTILE_ROW_Y 320   // fila de burbujas y esporas en images/enemies.png
#define BUBBLE_RANGE 128.f     // 4 tiles
#define POP_TIME 250
#define BOMB_GRAVITY 0.3f
#define BOMB_MAX_FALL 6.f


enum ProjectileAnims
{
	FLY, POP
};


Projectile::Projectile()
{
	sprite = NULL;
}

Projectile::~Projectile()
{
	if(sprite != NULL)
		delete sprite;
}

// spritesheet: images/enemies.png para BUBBLE y SPORE, images/bomb.png para FALLING_BOMB
void Projectile::init(ProjectileKind kind, const glm::vec2 &center, const glm::vec2 &velocity, Texture *spritesheet, ShaderProgram &shaderProgram)
{
	this->kind = kind;
	pos = center - glm::vec2(16.f, 16.f);
	vel = velocity;
	travelled = 0.f;
	bPopping = bLanded = bFinished = false;
	popTime = 0;

	if(kind == FALLING_BOMB)
	{
		sprite = Sprite::createSprite(glm::ivec2(32, 32), glm::vec2(0.25f, 1.f), spritesheet, &shaderProgram);
		sprite->setNumberAnimations(2);
		sprite->setAnimationSpeed(FLY, 6);
		for(int i = 0; i < 4; i++)
			sprite->addKeyframe(FLY, glm::vec2(0.25f * i, 0.f));
		sprite->setAnimationSpeed(POP, 1);
		sprite->addKeyframe(POP, glm::vec2(0.f, 0.f));
	}
	else
	{
		glm::vec2 cell(32.f / SHEET_W, 32.f / SHEET_H);
		float y = PROJECTILE_ROW_Y / SHEET_H;
		sprite = Sprite::createSprite(glm::ivec2(32, 32), cell, spritesheet, &shaderProgram);
		sprite->setNumberAnimations(2);
		// burbuja: columnas 0-2 volando, 3-4 explotando; espora: columnas 5-6
		int first = (kind == BUBBLE) ? 0 : 5, frames = (kind == BUBBLE) ? 3 : 2;
		sprite->setAnimationSpeed(FLY, 8);
		for(int i = 0; i < frames; i++)
			sprite->addKeyframe(FLY, glm::vec2((first + i) * cell.x, y));
		sprite->setAnimationSpeed(POP, 1000 * 2 / POP_TIME);
		if(kind == BUBBLE)
		{
			sprite->addKeyframe(POP, glm::vec2(3.f * cell.x, y));
			sprite->addKeyframe(POP, glm::vec2(4.f * cell.x, y));
		}
		else
			sprite->addKeyframe(POP, glm::vec2(5.f * cell.x, y));
	}
	sprite->changeAnimation(FLY);
	sprite->setPosition(glm::floor(pos));
}

glm::ivec2 Projectile::hitboxPos() const
{
	if(kind == FALLING_BOMB)
		return glm::ivec2(glm::floor(pos)) + glm::ivec2(4, 4);
	return glm::ivec2(glm::floor(pos)) + glm::ivec2(9, 9);
}

glm::ivec2 Projectile::hitboxSize() const
{
	if(kind == FALLING_BOMB)
		return glm::ivec2(24, 28);
	return glm::ivec2(14, 14);
}

void Projectile::pop()
{
	if(bPopping)
		return;
	bPopping = true;
	popTime = POP_TIME;
	sprite->changeAnimation(POP);
}

void Projectile::update(int deltaTime, TileMap *map)
{
	sprite->update(deltaTime);
	if(bPopping)
	{
		if((popTime -= deltaTime) <= 0)
			bFinished = true;
		return;
	}

	if(kind == FALLING_BOMB)
	{
		vel.y = std::min(vel.y + BOMB_GRAVITY, BOMB_MAX_FALL);
		pos.y += vel.y;
		glm::ivec2 hb = hitboxPos();
		int hbY = hb.y;
		if(vel.y > 0.f && map->collisionMoveDown(hb, hitboxSize(), &hbY))
		{
			pos.y = float(hbY - 4);
			bLanded = true;
		}
		// se ha caido fuera del mapa
		if(pos.y > map->getMapSize().y * map->getTileSize())
			bFinished = true;
	}
	else
	{
		pos += vel;
		travelled += glm::length(vel);
		glm::ivec2 hb = hitboxPos();
		// las esporas (bolas de energia de Trent) atraviesan cualquier bloque;
		// solo desaparecen al salir del mapa o al tocar al jugador
		if(kind == BUBBLE && (map->collisionBox(hb, hitboxSize()) || travelled >= BUBBLE_RANGE))
			pop();
		glm::ivec2 mapPx = map->getMapSize() * map->getTileSize();
		if(hb.x < -32 || hb.y < -32 || hb.x > mapPx.x || hb.y > mapPx.y)
			bFinished = true;
	}
	sprite->setPosition(glm::floor(pos));
}

void Projectile::render()
{
	if(!bFinished)
		sprite->render();
}
