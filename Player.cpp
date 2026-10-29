#include <cmath>
#include <iostream>
#include <GL/glew.h>
#include "Player.h"
#include "Game.h"


#define JUMP_ANGLE_STEP 4
#define JUMP_HEIGHT 96
#define FALL_STEP 4
#define HITBOX_OFFSET_X 4
#define HITBOX_WIDTH 24
#define HITBOX_HEIGHT 32
#define CORNER_TOLERANCE 8 // px que se puede empujar al jugador para colarse por un hueco


enum PlayerAnims
{
	STAND_LEFT, STAND_RIGHT, MOVE_LEFT, MOVE_RIGHT, 
    JUMP_UP_LEFT, JUMP_FALL_LEFT, JUMP_UP_RIGHT, JUMP_FALL_RIGHT, DEATH

};


Player::Player()
{
	sprite = NULL;
	map = NULL;
}

Player::~Player()
{
	if (sprite != NULL)
		delete sprite;
}

glm::ivec2 Player::hitboxPos() const
{
	return glm::ivec2(posPlayer.x + HITBOX_OFFSET_X, posPlayer.y);
}

glm::ivec2 Player::hitboxSize() const
{
	return glm::ivec2(HITBOX_WIDTH, HITBOX_HEIGHT);
}

void Player::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
{
	lookingRight = true;
	bJumping = false;
	bRising = false;
	bOnGround = false;
	bDead = false;
	spritesheet.loadFromFile("images/player.png", TEXTURE_PIXEL_FORMAT_RGBA);
	sprite = Sprite::createSprite(glm::ivec2(32, 32), glm::vec2(0.25, 0.25), &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(9);
	
		sprite->setAnimationSpeed(STAND_LEFT, 8);
		sprite->addKeyframe(STAND_LEFT, glm::vec2(0.f, 0.f));
		
		sprite->setAnimationSpeed(STAND_RIGHT, 8);
		sprite->addKeyframe(STAND_RIGHT, glm::vec2(0.f, 0.25f));
		
		sprite->setAnimationSpeed(MOVE_LEFT, 11);
		sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.f, 0.f));
		sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.25f, 0.f));
		sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.5f, 0.f));
		
		sprite->setAnimationSpeed(MOVE_RIGHT, 11);
		sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.f, 0.25f));
		sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.25f, 0.25f));
		sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.5f, 0.25f));

// --- SALTO IZQUIERDA (3ª fila, Y = 0.5f) ---
    // Subir (Frame 0, Columna 0)
    	sprite->setAnimationSpeed(JUMP_UP_LEFT, 4);
    	sprite->addKeyframe(JUMP_UP_LEFT, glm::vec2(0.f, 0.5f));
    
    // Caer (Frame 1, Columna 1)
    	sprite->setAnimationSpeed(JUMP_FALL_LEFT, 4);
    	sprite->addKeyframe(JUMP_FALL_LEFT, glm::vec2(0.25f, 0.5f));

    // --- SALTO DERECHA (3ª fila, Y = 0.5f) ---
    // Subir (Frame 0, Columna 2)
    	sprite->setAnimationSpeed(JUMP_UP_RIGHT, 4);
    	sprite->addKeyframe(JUMP_UP_RIGHT, glm::vec2(0.5f, 0.5f));
    
    // Caer (Frame 1, Columna 3)
    	sprite->setAnimationSpeed(JUMP_FALL_RIGHT, 4);
    	sprite->addKeyframe(JUMP_FALL_RIGHT, glm::vec2(0.75f, 0.5f));

    // --- MUERTE (4º sprite de la 1ª y 2ª fila, alternando) ---
    	sprite->setAnimationSpeed(DEATH, 8);
    	sprite->addKeyframe(DEATH, glm::vec2(0.75f, 0.f));
    	sprite->addKeyframe(DEATH, glm::vec2(0.75f, 0.25f));
		
	sprite->changeAnimation(0);
	tileMapDispl = tileMapPos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
	
}

void Player::update(int deltaTime)
{
    sprite->update(deltaTime);

    // muerto: solo se reproduce la animacion de muerte, sin control ni gravedad
    if(bDead)
        return;

    glm::ivec2 hbSize(HITBOX_WIDTH, HITBOX_HEIGHT);
    bool bInAir = bJumping || !bOnGround;
    int speed = bJumping ? 3 : 2; // en el salto se avanza un poco mas rapido
    
    // --- MOVIMIENTO HORIZONTAL ---
    if(Game::instance().getKey(GLFW_KEY_LEFT))
    {
        lookingRight = false;
        if(!bInAir && sprite->animation() != MOVE_LEFT)
            sprite->changeAnimation(MOVE_LEFT);
        posPlayer.x -= speed;
        if(map->collisionMoveLeft(hitboxPos(), hbSize) && !(bInAir && nudgeVertical()))
        {
            posPlayer.x += speed;
            if(!bInAir)
                sprite->changeAnimation(STAND_LEFT);
        }
    }
    else if(Game::instance().getKey(GLFW_KEY_RIGHT))
    {
        lookingRight = true;
        if(!bInAir && sprite->animation() != MOVE_RIGHT)
            sprite->changeAnimation(MOVE_RIGHT);
        posPlayer.x += speed;
        if(map->collisionMoveRight(hitboxPos(), hbSize) && !(bInAir && nudgeVertical()))
        {
            posPlayer.x -= speed;
            if(!bInAir)
                sprite->changeAnimation(STAND_RIGHT);
        }
    }
    else if(!bInAir)
    {
        if(sprite->animation() == MOVE_LEFT)
            sprite->changeAnimation(STAND_LEFT);
        else if(sprite->animation() == MOVE_RIGHT)
            sprite->changeAnimation(STAND_RIGHT);
    }
    
    // --- MOVIMIENTO VERTICAL ---
    if(bRising)
    {
        // subida del salto: media parabola (de 0 a 90 graus)
        jumpAngle += JUMP_ANGLE_STEP;
        if(jumpAngle >= 90)
        {
            posPlayer.y = startY - JUMP_HEIGHT;
            bRising = false; // a partir de aqui cae con la gravedad normal
        }
        else
            posPlayer.y = int(startY - JUMP_HEIGHT * sin(3.14159f * jumpAngle / 180.f));

        // techo: si la cabeza entra en un tile solido, se recoloca debajo y empieza a caer
        int freeY = posPlayer.y;
        if(map->collisionMoveUp(hitboxPos(), hbSize, &posPlayer.y))
        {
            // si solo roza la esquina del techo, se le empuja de lado hacia el hueco
            posPlayer.y = freeY;
            if(!nudgeHorizontal())
            {
                map->collisionMoveUp(hitboxPos(), hbSize, &posPlayer.y);
                bRising = false;
            }
        }

        bOnGround = false;
    }
    else
    {
        // caida: la misma velocidad tanto si viene de un salto como si no
        posPlayer.y += FALL_STEP;
        bOnGround = map->collisionMoveDown(hitboxPos(), hbSize, &posPlayer.y);

        if(bOnGround)
        {
            bJumping = false;
            if(Game::instance().getKey(GLFW_KEY_UP))
            {
                bJumping = true;
                bRising = true;
                bOnGround = false;
                jumpAngle = 0;
                startY = posPlayer.y;
            }
        }
    }

    // --- ANIMACION ---
    int targetAnim = -1;
    if(bRising)
        targetAnim = lookingRight ? JUMP_UP_RIGHT : JUMP_UP_LEFT;
    else if(!bOnGround)
        targetAnim = lookingRight ? JUMP_FALL_RIGHT : JUMP_FALL_LEFT;
    else if(sprite->animation() == JUMP_FALL_LEFT || sprite->animation() == JUMP_FALL_RIGHT ||
            sprite->animation() == JUMP_UP_LEFT || sprite->animation() == JUMP_UP_RIGHT)
        targetAnim = lookingRight ? STAND_RIGHT : STAND_LEFT; // acaba de aterrizar
    
    if(targetAnim != -1 && sprite->animation() != targetAnim)
        sprite->changeAnimation(targetAnim);
    
    sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}


bool Player::nudgeHorizontal()
{
    glm::ivec2 hbSize(HITBOX_WIDTH, HITBOX_HEIGHT);
    for(int d = 1; d <= CORNER_TOLERANCE; d++)
        for(int s = -1; s <= 1; s += 2)
            if(!map->collisionBox(hitboxPos() + glm::ivec2(s * d, 0), hbSize))
            {
                posPlayer.x += s * d;
                return true;
            }
    return false;
}

// Correccion de esquina lateral: en el aire, si al chocar con una pared basta
// con subir o bajar unos pixeles para entrar en un hueco, se hace.
bool Player::nudgeVertical()
{
    glm::ivec2 hbSize(HITBOX_WIDTH, HITBOX_HEIGHT);
    for(int d = 1; d <= CORNER_TOLERANCE; d++)
        for(int s = -1; s <= 1; s += 2)
            if(!map->collisionBox(hitboxPos() + glm::ivec2(0, s * d), hbSize))
            {
                posPlayer.y += s * d;
                startY += s * d; // la subida del salto sigue la curva desde la nueva altura
                return true;
            }
    return false;
}

void Player::kill()
{
    if(bDead)
        return;
    bDead = true;
    bJumping = false;
    bRising = false;
    sprite->changeAnimation(DEATH);
}

void Player::respawn(const glm::vec2 &pos)
{
    bDead = false;
    bJumping = false;
    bRising = false;
    bOnGround = false;
    lookingRight = true;
    sprite->changeAnimation(STAND_RIGHT);
    setPosition(pos);
}

void Player::render()
{
	sprite->render();
}

void Player::setTileMap(TileMap *tileMap)
{
	map = tileMap;
}

void Player::setPosition(const glm::vec2 &pos)
{
	posPlayer = pos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}




