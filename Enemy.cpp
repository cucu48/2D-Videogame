#include <cmath>
#include <cstdlib>
#include <algorithm>
#include "Enemy.h"


// images/enemies.png mide 256x512. Celdas de 32x32 salvo Elekin Man (32x64).
#define SHEET_W 256.f
#define SHEET_H 512.f
#define LEAF_ROW_Y 224
#define ELEKIN_ROW_Y 256

#define SLOW_SPEED 0.5f       // px por frame ("se mueve despacio")
#define NORMAL_SPEED 1.f
#define DASH_SPEED 4          // embestida de Eyti
#define HOP_RISE_SPEED 1.5f   // Springchicken subiendo
#define HOP_FORWARD_SPEED 0.75f
#define LEAF_SPEED 0.8f
#define ENEMY_FALL_STEP 6
#define JUMP_INTERVAL 1500
#define JUMP_VELOCITY -5.f
#define GRAVITY 0.3f
#define FLY_AMPLITUDE 8.f     // Cloudy: cuanto ondula arriba y abajo

#define STEP_PAUSE 500        // Jacky: pausa cada 2 tiles
#define TURN_PAUSE 600        // pausa antes de darse la vuelta
#define ALERT_TIME 400        // Eyti: pausa antes de embestir
#define DASH_REST 700         // Eyti: pausa despues de embestir
#define HOP_REST 300          // Springchicken: pausa al aterrizar
#define SHOT_COOLDOWN 2000    // Cloudy: tiempo entre burbujas
#define SHOT_RANGE_TILES 4
#define BOLT_TIME 3000        // Elekin Man: duracion del rayo
#define BOLT_COOLDOWN 1500
#define BOLT_TILES 2
#define LEAF_LIFETIME 10000
#define DYING_TIME 600


enum EnemyAnims
{
	WALK_LEFT, WALK_RIGHT, DIE, ATTACK
};

enum EnemyStates
{
	S_WALK, S_PAUSE, S_TURN, S_ALERT, S_DASH, S_REST, S_RISE, S_FORWARD, S_DROP, S_SHOOT
};



Enemy::Enemy()
{
	sprite = NULL;
	boltSprite = NULL;
	map = NULL;
}

Enemy::~Enemy()
{
	if(sprite != NULL)
		delete sprite;
	if(boltSprite != NULL)
		delete boltSprite;
}

void Enemy::init(int type, const glm::ivec2 &tileMapPos, Texture *spritesheet, ShaderProgram &shaderProgram)
{
	this->type = type;
	tileMapDispl = tileMapPos;
	direction = -1;
	velY = 0.f;
	subPixel = 0.f;
	bOnGround = false;
	state = (type == SPRINGCHICKEN) ? S_DROP : S_WALK;
	stateTime = 0;
	timer = (type == GREEN || type == ORANGE || type == BLACK) ? JUMP_INTERVAL : 0;
	walked = 0;
	groundY = 0;
	flyTime = 0.f;
	shotCooldown = 0;
	bShot = false;
	lifeTime = LEAF_LIFETIME;
	bDying = false;
	dyingTime = 0;

	// columnas: 0-1 izquierda, 2-3 derecha, 4-5 morir (Elekin: 0-1 flotar, 2 cargar, 3-4 morir, 5-6 rayo)
	glm::vec2 cell(32.f / SHEET_W, 32.f / SHEET_H);
	float rowY = type * 32.f / SHEET_H;
	size = glm::ivec2(32, 32);
	if(type == LEAF)
		rowY = LEAF_ROW_Y / SHEET_H;
	if(type == ELEKIN)
	{
		size = glm::ivec2(32, 64);
		cell.y = 64.f / SHEET_H;
		rowY = ELEKIN_ROW_Y / SHEET_H;
	}
	sprite = Sprite::createSprite(size, cell, spritesheet, &shaderProgram);
	sprite->setNumberAnimations(4);
	if(type == ELEKIN)
	{
		for(int anim = WALK_LEFT; anim <= WALK_RIGHT; anim++)
		{
			sprite->setAnimationSpeed(anim, 3);
			sprite->addKeyframe(anim, glm::vec2(0.f, rowY));
			sprite->addKeyframe(anim, glm::vec2(cell.x, rowY));
		}
		sprite->setAnimationSpeed(ATTACK, 8);
		sprite->addKeyframe(ATTACK, glm::vec2(2.f * cell.x, rowY));
		sprite->addKeyframe(ATTACK, glm::vec2(0.f, rowY));
		sprite->setAnimationSpeed(DIE, 1000 * 2 / DYING_TIME);
		sprite->addKeyframe(DIE, glm::vec2(3.f * cell.x, rowY));
		sprite->addKeyframe(DIE, glm::vec2(4.f * cell.x, rowY));

		// rayo: 32x64 (2 tiles), parpadea entre 2 frames
		boltSprite = Sprite::createSprite(glm::ivec2(32, 64), cell, spritesheet, &shaderProgram);
		boltSprite->setNumberAnimations(1);
		boltSprite->setAnimationSpeed(0, 12);
		boltSprite->addKeyframe(0, glm::vec2(5.f * cell.x, rowY));
		boltSprite->addKeyframe(0, glm::vec2(6.f * cell.x, rowY));
		boltSprite->changeAnimation(0);
	}
	else
	{
		sprite->setAnimationSpeed(WALK_LEFT, 4);
		sprite->addKeyframe(WALK_LEFT, glm::vec2(0.f, rowY));
		sprite->addKeyframe(WALK_LEFT, glm::vec2(cell.x, rowY));
		sprite->setAnimationSpeed(WALK_RIGHT, 4);
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(2.f * cell.x, rowY));
		sprite->addKeyframe(WALK_RIGHT, glm::vec2(3.f * cell.x, rowY));
		// desaparecer: 2 frames y despues ya no se dibuja
		sprite->setAnimationSpeed(DIE, 1000 * 2 / DYING_TIME);
		sprite->addKeyframe(DIE, glm::vec2(4.f * cell.x, rowY));
		sprite->addKeyframe(DIE, glm::vec2(5.f * cell.x, rowY));
		sprite->setAnimationSpeed(ATTACK, 1);
		sprite->addKeyframe(ATTACK, glm::vec2(0.f, rowY));
	}
	sprite->changeAnimation(WALK_LEFT);
}

void Enemy::setPosition(const glm::ivec2 &pos)
{
	posEnemy = pos;
	leafPos = glm::vec2(pos);
	flyBaseY = pos.y;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x), float(tileMapDispl.y + posEnemy.y)));
}

void Enemy::setDirection(int dir)
{
	direction = (dir < 0) ? -1 : 1;
	updateAnimation();
}

void Enemy::updateAnimation()
{
	if(bDying)
		return;
	int anim = (direction < 0) ? WALK_LEFT : WALK_RIGHT;
	if(type == ELEKIN && state == S_SHOOT)
		anim = ATTACK;
	if(sprite->animation() != anim)
		sprite->changeAnimation(anim);
}

glm::ivec2 Enemy::hitboxPos() const
{
	if(type == ELEKIN)
		return posEnemy + glm::ivec2(6, 12);
	if(type == LEAF)
		return posEnemy + glm::ivec2(6, 6);
	return posEnemy + glm::ivec2(4, 4);
}

glm::ivec2 Enemy::hitboxSize() const
{
	if(type == ELEKIN)
		return glm::ivec2(20, 50);
	if(type == LEAF)
		return glm::ivec2(20, 20);
	return glm::ivec2(24, 28);
}

void Enemy::kill()
{
	if(bDying)
		return;
	bDying = true;
	dyingTime = DYING_TIME;
	sprite->changeAnimation(DIE);
}

void Enemy::update(int deltaTime, const glm::ivec2 &playerCenter, bool playerAlive)
{
	sprite->update(deltaTime);
	if(boltSprite != NULL)
		boltSprite->update(deltaTime);

	if(bDying)
	{
		dyingTime -= deltaTime;
		return;
	}

	switch(type)
	{
	case JACKY: updateJacky(deltaTime); break;
	case EYTI: updateEyti(deltaTime, playerCenter, playerAlive); break;
	case CLOUDY: updateCloudy(deltaTime, playerCenter, playerAlive); break;
	case SPRINGCHICKEN: updateSpringchicken(deltaTime); break;
	case ELEKIN: updateElekin(deltaTime, playerCenter, playerAlive); break;
	case LEAF: updateLeaf(deltaTime, playerCenter); break;
	default: updateJumper(deltaTime); break;
	}

	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x), float(tileMapDispl.y + posEnemy.y)));
	if(boltSprite != NULL)
		boltSprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x), float(tileMapDispl.y + posEnemy.y + 60)));
}

void Enemy::render()
{
	if(isGone())
		return;
	sprite->render();
	if(type == ELEKIN && state == S_SHOOT && !bDying)
		boltSprite->render();
}

void Enemy::turnAround()
{
	setDirection(-direction);
}

int Enemy::step(float speed)
{
	subPixel += speed;
	int px = int(subPixel);
	subPixel -= px;
	return px;
}

bool Enemy::moveX(int dx)
{
	posEnemy.x += dx;
	bool wall = (dx > 0) ? map->collisionMoveRight(hitboxPos(), hitboxSize())
	                     : map->collisionMoveLeft(hitboxPos(), hitboxSize());
	if(wall)
		posEnemy.x -= dx;
	return !wall;
}

// gravedad: si no tiene suelo debajo, cae. Devuelve true si esta en el suelo
bool Enemy::fall()
{
	glm::ivec2 hb = hitboxPos();
	hb.y += ENEMY_FALL_STEP;
	int hbY = hb.y;
	bOnGround = map->collisionMoveDown(hb, hitboxSize(), &hbY);
	posEnemy.y = hbY - (hitboxPos().y - posEnemy.y);
	return bOnGround;
}

// true si hay suelo justo delante de los pies (para no caerse por los bordes)
bool Enemy::groundAhead() const
{
	int tileSize = map->getTileSize();
	glm::ivec2 pos = hitboxPos(), hbSize = hitboxSize();
	int frontX = (direction > 0) ? pos.x + hbSize.x : pos.x - 1;
	int feetY = pos.y + hbSize.y;
	return map->isSolid(int(floor(float(frontX) / tileSize)), int(floor(float(feetY) / tileSize)));
}

// El jugador esta en la misma fila y no hay paredes ni bombas en medio
bool Enemy::seesPlayer(const glm::ivec2 &playerCenter) const
{
	int tileSize = map->getTileSize();
	glm::ivec2 center = hitboxPos() + hitboxSize() / 2;
	if(abs(playerCenter.y - center.y) > tileSize / 2)
		return false;
	int row = center.y / tileSize;
	int x0 = std::min(center.x, playerCenter.x) / tileSize, x1 = std::max(center.x, playerCenter.x) / tileSize;
	for(int x = x0; x <= x1; x++)
		if(map->isSolid(x, row))
			return false;
	return true;
}

// Jacky: anda despacio y se para cada 2 tiles. En un borde, pared o bomba se
// para un momento y se da la vuelta.
void Enemy::updateJacky(int deltaTime)
{
	if(!fall())
		return;
	switch(state)
	{
	case S_WALK:
	{
		int dx = step(SLOW_SPEED);
		if(dx == 0)
			break;
		if(!moveX(dx * direction) || !groundAhead())
		{
			state = S_TURN;
			stateTime = TURN_PAUSE;
		}
		else if((walked += dx) >= 2 * map->getTileSize())
		{
			walked = 0;
			state = S_PAUSE;
			stateTime = STEP_PAUSE;
		}
		break;
	}
	case S_PAUSE:
		if((stateTime -= deltaTime) <= 0)
			state = S_WALK;
		break;
	case S_TURN:
		if((stateTime -= deltaTime) <= 0)
		{
			turnAround();
			walked = 0;
			state = S_WALK;
		}
		break;
	}
}

// Eyti: anda despacio y se da la vuelta (con pausa) en bordes, paredes y
// bombas. Si ve al jugador en su fila, se para y embiste hacia el.
void Enemy::updateEyti(int deltaTime, const glm::ivec2 &playerCenter, bool playerAlive)
{
	if(!fall())
		return;
	switch(state)
	{
	case S_WALK:
	{
		// solo embiste si el jugador esta delante (en la direccion en la que mira)
		glm::ivec2 center = hitboxPos() + hitboxSize() / 2;
		bool inFront = (direction > 0) ? playerCenter.x > center.x : playerCenter.x < center.x;
		if(playerAlive && inFront && seesPlayer(playerCenter))
		{
			state = S_ALERT;
			stateTime = ALERT_TIME;
			break;
		}
		int dx = step(SLOW_SPEED);
		if(dx != 0 && (!moveX(dx * direction) || !groundAhead()))
		{
			state = S_TURN;
			stateTime = TURN_PAUSE;
		}
		break;
	}
	case S_TURN:
		if((stateTime -= deltaTime) <= 0)
		{
			turnAround();
			state = S_WALK;
		}
		break;
	case S_ALERT:
		if((stateTime -= deltaTime) <= 0)
			state = S_DASH;
		break;
	case S_DASH:
		for(int i = 0; i < DASH_SPEED; i++)
			if(!moveX(direction) || !groundAhead())
			{
				state = S_REST;
				stateTime = DASH_REST;
				break;
			}
		break;
	case S_REST:
		if((stateTime -= deltaTime) <= 0)
			state = S_WALK;
		break;
	}
}

// Tipos 2-4: andan y cada cierto tiempo dan un saltito
void Enemy::updateJumper(int deltaTime)
{
	timer -= deltaTime;
	if(bOnGround && timer <= 0)
	{
		velY = JUMP_VELOCITY;
		bOnGround = false;
		timer = JUMP_INTERVAL;
	}

	if(bOnGround || velY == 0.f)
	{
		if(!fall())
			return;
		if(!moveX(direction) || !groundAhead())
			turnAround();
		return;
	}

	// en el aire: avanza y aplica la gravedad
	if(!moveX(direction))
		turnAround();
	int offsetY = hitboxPos().y - posEnemy.y;
	velY = std::min(velY + GRAVITY, float(ENEMY_FALL_STEP));
	posEnemy.y += int(round(velY));
	int hbY = posEnemy.y + offsetY;
	if(velY < 0)
	{
		if(map->collisionMoveUp(hitboxPos(), hitboxSize(), &hbY))
		{
			posEnemy.y = hbY - offsetY;
			velY = 0.f;
		}
	}
	else if(map->collisionMoveDown(hitboxPos(), hitboxSize(), &hbY))
	{
		posEnemy.y = hbY - offsetY;
		velY = 0.f;
		bOnGround = true;
	}
}

// Cloudy: vuela en horizontal ondulando arriba y abajo (sin gravedad) y se da
// la vuelta al chocar con una pared o una bomba. Si el jugador esta cerca le
// dispara burbujas.
void Enemy::updateCloudy(int deltaTime, const glm::ivec2 &playerCenter, bool playerAlive)
{
	flyTime += deltaTime;
	int dx = step(NORMAL_SPEED);
	if(dx != 0 && !moveX(dx * direction))
		turnAround();
	int oldY = posEnemy.y;
	posEnemy.y = flyBaseY + int(FLY_AMPLITUDE * sin(flyTime / 300.f));
	if(map->collisionBox(hitboxPos(), hitboxSize()))
		posEnemy.y = oldY;

	shotCooldown -= deltaTime;
	glm::vec2 center = glm::vec2(hitboxPos() + hitboxSize() / 2);
	glm::vec2 toPlayer = glm::vec2(playerCenter) - center;
	float dist = glm::length(toPlayer);
	if(playerAlive && shotCooldown <= 0 && dist > 0.f && dist <= SHOT_RANGE_TILES * map->getTileSize())
	{
		bShot = true;
		shotDir = toPlayer / dist;
		shotCooldown = SHOT_COOLDOWN;
	}
}

bool Enemy::takeShot(glm::vec2 &pos, glm::vec2 &dir)
{
	if(!bShot)
		return false;
	bShot = false;
	pos = glm::vec2(hitboxPos() + hitboxSize() / 2);
	dir = shotDir;
	return true;
}

// Springchicken: sube un tile, avanza un tile y cae. Se da la vuelta al
// chocar con una pared o una bomba, o en el borde de su trozo de suelo.
void Enemy::updateSpringchicken(int deltaTime)
{
	int tileSize = map->getTileSize();
	switch(state)
	{
	case S_REST:
		if(fall() && (stateTime -= deltaTime) <= 0)
		{
			state = S_RISE;
			walked = 0;
		}
		break;
	case S_RISE:
	{
		int n = step(HOP_RISE_SPEED);
		for(int i = 0; i < n && state == S_RISE; i++)
		{
			posEnemy.y -= 1;
			if(map->collisionBox(hitboxPos(), hitboxSize()))
			{
				posEnemy.y += 1; // techo: avanza desde aqui
				walked = tileSize;
			}
			else
				walked++;
			if(walked >= tileSize)
			{
				state = S_FORWARD;
				walked = 0;
			}
		}
		break;
	}
	case S_FORWARD:
	{
		int n = step(HOP_FORWARD_SPEED);
		for(int i = 0; i < n && state == S_FORWARD; i++)
		{
			// no se sale del trozo de suelo donde estaba: si delante no hay suelo, se da la vuelta
			glm::ivec2 hb = hitboxPos();
			int frontX = (direction > 0) ? hb.x + hitboxSize().x : hb.x - 1;
			if(!map->isSolid(frontX / tileSize, groundY / tileSize))
			{
				turnAround();
				state = S_DROP;
			}
			else if(!moveX(direction))
			{
				turnAround();
				state = S_DROP;
			}
			else if(++walked >= tileSize)
				state = S_DROP;
		}
		break;
	}
	case S_DROP:
	default:
		if(fall())
		{
			groundY = hitboxPos().y + hitboxSize().y; // primera fila de pixeles del suelo

			state = S_REST;
			stateTime = HOP_REST;
		}
		break;
	}
}

// Elekin Man: flota despacio a izquierda y derecha (sin gravedad). Si el
// jugador esta justo debajo, se para y suelta un rayo de 2 tiles durante 3 s.
void Enemy::updateElekin(int deltaTime, const glm::ivec2 &playerCenter, bool playerAlive)
{
	timer -= deltaTime;
	if(state == S_SHOOT)
	{
		if((stateTime -= deltaTime) <= 0)
		{
			state = S_WALK;
			timer = BOLT_COOLDOWN;
			updateAnimation();
		}
		return;
	}

	int dx = step(SLOW_SPEED);
	if(dx != 0 && !moveX(dx * direction))
		turnAround();

	glm::ivec2 pos = hitboxPos(), hbSize = hitboxSize();
	int bottom = pos.y + hbSize.y;
	bool below = playerCenter.x >= pos.x - 4 && playerCenter.x <= pos.x + hbSize.x + 4 &&
	             playerCenter.y > bottom && playerCenter.y <= bottom + BOLT_TILES * map->getTileSize() + 16;
	if(playerAlive && timer <= 0 && below)
	{
		state = S_SHOOT;
		stateTime = BOLT_TIME;
		updateAnimation();
	}
}

bool Enemy::attackBox(glm::ivec2 &pos, glm::ivec2 &boxSize) const
{
	if(type != ELEKIN || state != S_SHOOT || bDying)
		return false;
	int tileSize = map->getTileSize();
	glm::ivec2 hb = hitboxPos();
	int centerX = hb.x + hitboxSize().x / 2;
	int top = hb.y + hitboxSize().y;
	// el rayo llega hasta 2 tiles o hasta la primera pared
	int length = 0;
	while(length < BOLT_TILES * tileSize && !map->isWall(centerX / tileSize, (top + length) / tileSize))
		length += 4;
	pos = glm::ivec2(centerX - 6, top);
	boxSize = glm::ivec2(12, length);
	return length > 0;
}

// Leaf: persigue al jugador atravesando paredes y desaparece al cabo de un rato
void Enemy::updateLeaf(int deltaTime, const glm::ivec2 &playerCenter)
{
	if((lifeTime -= deltaTime) <= 0)
	{
		kill();
		return;
	}
	glm::vec2 center = leafPos + glm::vec2(size) / 2.f;
	glm::vec2 toPlayer = glm::vec2(playerCenter) - center;
	float dist = glm::length(toPlayer);
	if(dist > 1.f)
		leafPos += toPlayer / dist * LEAF_SPEED;
	posEnemy = glm::ivec2(glm::floor(leafPos));
	int dir = (toPlayer.x < 0.f) ? -1 : 1;
	if(dir != direction && fabs(toPlayer.x) > 4.f)
		setDirection(dir);
}
