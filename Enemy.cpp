#include <cmath>
#include "Enemy.h"


#define ENEMY_SPEED 1
#define ENEMY_FALL_STEP 6
#define HITBOX_OFFSET_X 4
#define HITBOX_OFFSET_Y 4
#define HITBOX_WIDTH 24
#define HITBOX_HEIGHT 28
#define JUMP_INTERVAL 1500
#define JUMP_VELOCITY -5.f
#define GRAVITY 0.3f
#define FLY_AMPLITUDE 8.f
#define DYING_TIME 600


enum EnemyAnims
{
	WALK_LEFT, WALK_RIGHT, DIE
};

// comportamiento de cada fila de images/enemies.png
static const EnemyBehaviour BEHAVIOURS[7] = { WALKER, WALKER, JUMPER, JUMPER, JUMPER, FLYER, WALKER };


Enemy::Enemy()
{
	sprite = NULL;
	map = NULL;
}

Enemy::~Enemy()
{
	if(sprite != NULL)
		delete sprite;
}

void Enemy::init(int type, const glm::ivec2 &tileMapPos, Texture *spritesheet, ShaderProgram &shaderProgram)
{
	behaviour = BEHAVIOURS[type];
	tileMapDispl = tileMapPos;
	direction = -1;
	velY = 0.f;
	bOnGround = false;
	jumpTimer = JUMP_INTERVAL;
	flyTime = 0.f;
	bDying = false;
	dyingTime = 0;

	// images/enemies.png: 8x8 celdas de 32x32, una fila por tipo de enemigo
	float row = type / 8.f;
	sprite = Sprite::createSprite(glm::ivec2(32, 32), glm::vec2(1.f / 8.f, 1.f / 8.f), spritesheet, &shaderProgram);
	sprite->setNumberAnimations(3);

	sprite->setAnimationSpeed(WALK_LEFT, 4);
	sprite->addKeyframe(WALK_LEFT, glm::vec2(0.f / 8.f, row));
	sprite->addKeyframe(WALK_LEFT, glm::vec2(1.f / 8.f, row));

	sprite->setAnimationSpeed(WALK_RIGHT, 4);
	sprite->addKeyframe(WALK_RIGHT, glm::vec2(2.f / 8.f, row));
	sprite->addKeyframe(WALK_RIGHT, glm::vec2(3.f / 8.f, row));

	// desaparecer: 2 frames y despues ya no se dibuja
	sprite->setAnimationSpeed(DIE, 1000 * 2 / DYING_TIME);
	sprite->addKeyframe(DIE, glm::vec2(4.f / 8.f, row));
	sprite->addKeyframe(DIE, glm::vec2(5.f / 8.f, row));

	sprite->changeAnimation(WALK_LEFT);
}

void Enemy::setPosition(const glm::ivec2 &pos)
{
	posEnemy = pos;
	flyBaseY = pos.y;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x), float(tileMapDispl.y + posEnemy.y)));
}

void Enemy::setDirection(int dir)
{
	direction = (dir < 0) ? -1 : 1;
	sprite->changeAnimation(direction < 0 ? WALK_LEFT : WALK_RIGHT);
}

glm::ivec2 Enemy::hitboxPos() const
{
	return glm::ivec2(posEnemy.x + HITBOX_OFFSET_X, posEnemy.y + HITBOX_OFFSET_Y);
}

glm::ivec2 Enemy::hitboxSize() const
{
	return glm::ivec2(HITBOX_WIDTH, HITBOX_HEIGHT);
}

void Enemy::kill()
{
	if(bDying)
		return;
	bDying = true;
	dyingTime = DYING_TIME;
	sprite->changeAnimation(DIE);
}

void Enemy::update(int deltaTime)
{
	sprite->update(deltaTime);

	if(bDying)
	{
		dyingTime -= deltaTime;
		return;
	}

	if(behaviour == FLYER)
	{
		flyTime += deltaTime;
		updateFly();
	}
	else if(behaviour == JUMPER)
	{
		jumpTimer -= deltaTime;
		updateJump();
	}
	else
		updateWalk();

	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x), float(tileMapDispl.y + posEnemy.y)));
}

void Enemy::render()
{
	if(!isGone())
		sprite->render();
}

void Enemy::turnAround()
{
	setDirection(-direction);
}

// true si hay suelo justo delante de los pies (para no caerse por los bordes)
bool Enemy::groundAhead() const
{
	int tileSize = map->getTileSize();
	glm::ivec2 pos = hitboxPos(), size = hitboxSize();
	int frontX = (direction > 0) ? pos.x + size.x : pos.x - 1;
	int feetY = pos.y + size.y;
	return map->isSolid(int(floor(float(frontX) / tileSize)), int(floor(float(feetY) / tileSize)));
}

void Enemy::updateWalk()
{
	// gravedad: si no tiene suelo debajo, cae
	int newY = posEnemy.y + ENEMY_FALL_STEP;
	glm::ivec2 hb(posEnemy.x + HITBOX_OFFSET_X, newY + HITBOX_OFFSET_Y);
	int hbY = hb.y;
	bOnGround = map->collisionMoveDown(hb, hitboxSize(), &hbY);
	posEnemy.y = hbY - HITBOX_OFFSET_Y;
	if(!bOnGround)
		return;

	// andar y darse la vuelta en paredes y bordes
	posEnemy.x += direction * ENEMY_SPEED;
	bool wall = (direction > 0) ? map->collisionMoveRight(hitboxPos(), hitboxSize())
	                            : map->collisionMoveLeft(hitboxPos(), hitboxSize());
	if(wall)
	{
		posEnemy.x -= direction * ENEMY_SPEED;
		turnAround();
	}
	else if(!groundAhead())
		turnAround();
}

void Enemy::updateJump()
{
	if(bOnGround && jumpTimer <= 0)
	{
		// saltito
		velY = JUMP_VELOCITY;
		bOnGround = false;
		jumpTimer = JUMP_INTERVAL;
	}

	if(bOnGround)
	{
		updateWalk();
		return;
	}

	// en el aire: avanza y aplica la gravedad
	posEnemy.x += direction * ENEMY_SPEED;
	bool wall = (direction > 0) ? map->collisionMoveRight(hitboxPos(), hitboxSize())
	                            : map->collisionMoveLeft(hitboxPos(), hitboxSize());
	if(wall)
	{
		posEnemy.x -= direction * ENEMY_SPEED;
		turnAround();
	}

	velY = std::min(velY + GRAVITY, float(ENEMY_FALL_STEP));
	posEnemy.y += int(round(velY));
	int hbY = posEnemy.y + HITBOX_OFFSET_Y;
	if(velY < 0)
	{
		if(map->collisionMoveUp(hitboxPos(), hitboxSize(), &hbY))
		{
			posEnemy.y = hbY - HITBOX_OFFSET_Y;
			velY = 0.f;
		}
	}
	else if(map->collisionMoveDown(hitboxPos(), hitboxSize(), &hbY))
	{
		posEnemy.y = hbY - HITBOX_OFFSET_Y;
		velY = 0.f;
		bOnGround = true;
	}
}

void Enemy::updateFly()
{
	// vuela en horizontal y ondula arriba y abajo; se da la vuelta en las paredes
	posEnemy.x += direction * ENEMY_SPEED;
	bool wall = (direction > 0) ? map->collisionMoveRight(hitboxPos(), hitboxSize())
	                            : map->collisionMoveLeft(hitboxPos(), hitboxSize());
	if(wall)
	{
		posEnemy.x -= direction * ENEMY_SPEED;
		turnAround();
	}

	int oldY = posEnemy.y;
	posEnemy.y = flyBaseY + int(FLY_AMPLITUDE * sin(flyTime / 300.f));
	if(map->collisionBox(hitboxPos(), hitboxSize()))
		posEnemy.y = oldY;
}
