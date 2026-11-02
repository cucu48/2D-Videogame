#include <cstdlib>
#include <ctime>
#include "Trent.h"


#define SHEET_W 256.f
#define SHEET_H 512.f
#define TRENT_ROW_Y 352        // frames de Trent en images/enemies.png (64x64)
#define TRENT_DIE_ROW_Y 416

#define TRENT_SPEED 1.5f       // un poco mas lento que el jugador
#define TRENT_HITS 5
#define INVULNERABLE_TIME 1500 // tras recibir un golpe parpadea y no le afectan las explosiones
#define FIRST_ATTACK_TIME 3000
#define ATTACK_INTERVAL 3500   // + hasta 2 s aleatorios
#define WINDUP_TIME 800        // se para antes de atacar
#define RECOVER_TIME 600       // y un poco despues
#define DYING_TIME 1200


enum TrentAnims
{
	T_ANIM_WALK, T_ANIM_WINDUP, T_ANIM_SHAKE, T_ANIM_DIE
};

enum TrentStates
{
	T_WALK, T_DESCEND, T_ASCEND, T_WINDUP, T_RECOVER
};


Trent::Trent()
{
	sprite = NULL;
	map = NULL;
}

Trent::~Trent()
{
	if(sprite != NULL)
		delete sprite;
}

void Trent::init(const glm::ivec2 &tileMapPos, Texture *spritesheet, ShaderProgram &shaderProgram)
{
	srand(unsigned(time(NULL))); // ataques distintos en cada partida
	tileMapDispl = tileMapPos;
	direction = 1;
	subPixel = 0.f;
	state = T_WALK;
	stateTime = 0;
	attackTimer = FIRST_ATTACK_TIME;
	chosenAttack = pendingAttack = TRENT_NO_ATTACK;
	bPlayerWasInFront = false;
	hitsLeft = TRENT_HITS;
	invulnerableTime = 0;
	bDying = false;
	dyingTime = 0;

	glm::vec2 cell(64.f / SHEET_W, 64.f / SHEET_H);
	float y = TRENT_ROW_Y / SHEET_H, dieY = TRENT_DIE_ROW_Y / SHEET_H;
	sprite = Sprite::createSprite(glm::ivec2(64, 64), cell, spritesheet, &shaderProgram);
	sprite->setNumberAnimations(4);
	sprite->setAnimationSpeed(T_ANIM_WALK, 3);
	sprite->addKeyframe(T_ANIM_WALK, glm::vec2(0.f, y));
	sprite->addKeyframe(T_ANIM_WALK, glm::vec2(cell.x, y));
	sprite->setAnimationSpeed(T_ANIM_WINDUP, 1);
	sprite->addKeyframe(T_ANIM_WINDUP, glm::vec2(2.f * cell.x, y));
	sprite->setAnimationSpeed(T_ANIM_SHAKE, 10); // sacude las ramas
	sprite->addKeyframe(T_ANIM_SHAKE, glm::vec2(3.f * cell.x, y));
	sprite->addKeyframe(T_ANIM_SHAKE, glm::vec2(0.f, y));
	sprite->setAnimationSpeed(T_ANIM_DIE, 1000 * 2 / DYING_TIME);
	sprite->addKeyframe(T_ANIM_DIE, glm::vec2(0.f, dieY));
	sprite->addKeyframe(T_ANIM_DIE, glm::vec2(cell.x, dieY));
	sprite->changeAnimation(T_ANIM_WALK);
}

void Trent::setPosition(const glm::ivec2 &position)
{
	pos = position;
	topY = position.y;
	sprite->setPosition(glm::vec2(tileMapDispl + pos));
}

glm::ivec2 Trent::hitboxPos() const
{
	return pos + glm::ivec2(8, 6);
}

glm::ivec2 Trent::hitboxSize() const
{
	return glm::ivec2(48, 56);
}

void Trent::changeAnim(int anim)
{
	if(sprite->animation() != anim)
		sprite->changeAnimation(anim);
}

int Trent::step(float speed)
{
	subPixel += speed;
	int px = int(subPixel);
	subPixel -= px;
	return px;
}

bool Trent::hit()
{
	if(bDying || invulnerableTime > 0)
		return false;
	invulnerableTime = INVULNERABLE_TIME;
	if(--hitsLeft <= 0)
	{
		bDying = true;
		dyingTime = DYING_TIME;
		changeAnim(T_ANIM_DIE);
	}
	return true;
}

void Trent::kill()
{
	if(bDying)
		return;
	hitsLeft = 0;
	bDying = true;
	dyingTime = DYING_TIME;
	changeAnim(T_ANIM_DIE);
}

TrentAttack Trent::takeAttack()
{
	TrentAttack attack = pendingAttack;
	pendingAttack = TRENT_NO_ATTACK;
	return attack;
}

void Trent::update(int deltaTime, const glm::ivec2 &playerCenter, bool playerAlive)
{
	sprite->update(deltaTime);
	if(invulnerableTime > 0)
		invulnerableTime -= deltaTime;
	if(bDying)
	{
		dyingTime -= deltaTime;
		return;
	}

	int tileSize = map->getTileSize();
	int mapW = map->getMapSize().x * tileSize, mapH = map->getMapSize().y * tileSize;
	int minX = tileSize, maxX = mapW - tileSize - 64;
	int bottomY = mapH - tileSize - 64;

	// el jugador pasa por delante (debajo de el)
	bool inFront = playerAlive && playerCenter.x >= pos.x + 8 && playerCenter.x <= pos.x + 56 && playerCenter.y > pos.y + 64;
	bool justCrossed = inFront && !bPlayerWasInFront;
	bPlayerWasInFront = inFront;

	switch(state)
	{
	case T_WALK:
	{
		changeAnim(T_ANIM_WALK);
		pos.x += direction * step(TRENT_SPEED);
		if(pos.x <= minX || pos.x >= maxX)
		{
			pos.x = glm::clamp(pos.x, minX, maxX);
			direction = -direction;
		}
		if(justCrossed && rand() % 2 == 0)
		{
			state = T_DESCEND;
			break;
		}
		if((attackTimer -= deltaTime) <= 0)
		{
			chosenAttack = TrentAttack(TRENT_LEAVES + rand() % 3);
			state = T_WINDUP;
			stateTime = WINDUP_TIME;
			changeAnim(chosenAttack == TRENT_BOMB_RAIN ? T_ANIM_SHAKE : T_ANIM_WINDUP);
		}
		break;
	}
	case T_DESCEND:
		pos.y += step(TRENT_SPEED);
		if(pos.y >= bottomY)
		{
			pos.y = bottomY;
			state = T_ASCEND;
		}
		break;
	case T_ASCEND:
		pos.y -= step(TRENT_SPEED);
		if(pos.y <= topY)
		{
			pos.y = topY;
			state = T_WALK;
		}
		break;
	case T_WINDUP:
		if((stateTime -= deltaTime) <= 0)
		{
			pendingAttack = chosenAttack;
			state = T_RECOVER;
			stateTime = RECOVER_TIME;
		}
		break;
	case T_RECOVER:
		if((stateTime -= deltaTime) <= 0)
		{
			state = T_WALK;
			attackTimer = ATTACK_INTERVAL + rand() % 2000;
		}
		break;
	}

	sprite->setPosition(glm::vec2(tileMapDispl + pos));
}

void Trent::render()
{
	if(isGone())
		return;
	// parpadea mientras es invulnerable
	if(!bDying && invulnerableTime > 0 && (invulnerableTime / 100) % 2 == 0)
		return;
	sprite->render();
}
