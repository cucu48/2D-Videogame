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


enum PlayerAnims
{
	STAND_LEFT, STAND_RIGHT, MOVE_LEFT, MOVE_RIGHT, 
    JUMP_UP_LEFT, JUMP_FALL_LEFT, JUMP_UP_RIGHT, JUMP_FALL_RIGHT

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

void Player::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
{
	lookingRight = true;
	bJumping = false;
	spritesheet.loadFromFile("images/player.png", TEXTURE_PIXEL_FORMAT_RGBA);
	sprite = Sprite::createSprite(glm::ivec2(32, 32), glm::vec2(0.25, 0.25), &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(8);
	
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
		
	sprite->changeAnimation(0);
	tileMapDispl = tileMapPos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
	
}

void Player::update(int deltaTime)
{
    sprite->update(deltaTime);
    
    // --- MOVIMIENTO HORIZONTAL (En suelo) ---
    if(!bJumping)
    {
        if(Game::instance().getKey(GLFW_KEY_LEFT))
        {
            lookingRight = false; // Actualizamos dirección
            if(sprite->animation() != MOVE_LEFT)
                sprite->changeAnimation(MOVE_LEFT);
            posPlayer.x -= 2;
            if(map->collisionMoveLeft(hitboxPos(), glm::ivec2(HITBOX_WIDTH, HITBOX_HEIGHT)))
            {
                posPlayer.x += 2;
                sprite->changeAnimation(STAND_LEFT);
            }
        }
        else if(Game::instance().getKey(GLFW_KEY_RIGHT))
        {
            lookingRight = true; // Actualizamos dirección
            if(sprite->animation() != MOVE_RIGHT)
                sprite->changeAnimation(MOVE_RIGHT);
            posPlayer.x += 2;
            if(map->collisionMoveRight(hitboxPos(), glm::ivec2(HITBOX_WIDTH, HITBOX_HEIGHT)))
            {
                posPlayer.x -= 2;
                sprite->changeAnimation(STAND_RIGHT);
            }
        }
        else
        {
            if(sprite->animation() == MOVE_LEFT)
                sprite->changeAnimation(STAND_LEFT);
            else if(sprite->animation() == MOVE_RIGHT)
                sprite->changeAnimation(STAND_RIGHT);
        }
    }
    
	//logica salto
    if(bJumping)
    {
jumpAngle += JUMP_ANGLE_STEP;

        if(Game::instance().getKey(GLFW_KEY_LEFT))
        {
            lookingRight = false;
            posPlayer.x -= 3;
			//if (jumpAngle < 86 || jumpAngle > 94) {
            	if(map->collisionMoveLeft(hitboxPos(), glm::ivec2(HITBOX_WIDTH, HITBOX_HEIGHT)))
            	{
                	posPlayer.x += 3; // Si choca con pared izquierda, frena
            	}
			//}
        }
        else if(Game::instance().getKey(GLFW_KEY_RIGHT))
        {
            lookingRight = true;
            posPlayer.x += 3;
            if(map->collisionMoveRight(hitboxPos(), glm::ivec2(HITBOX_WIDTH, HITBOX_HEIGHT)))
            {
                posPlayer.x -= 3; // Si choca con pared derecha, frena
            }
        }

        //canvi animacio de salt segons si cau o puja
        int targetAnim;
        if (jumpAngle < 90) {
            targetAnim = lookingRight ? JUMP_UP_RIGHT : JUMP_UP_LEFT;
        } else {
            targetAnim = lookingRight ? JUMP_FALL_RIGHT : JUMP_FALL_LEFT;
        }
        
        if (sprite->animation() != targetAnim)
            sprite->changeAnimation(targetAnim);

        // calcul parabola de salt 
        if(jumpAngle == 180)
        {
            bJumping = false;
            posPlayer.y = startY;
            sprite->changeAnimation(lookingRight ? STAND_RIGHT : STAND_LEFT);
        }
        else
        {
            posPlayer.y = int(startY - JUMP_HEIGHT * sin(3.14159f * jumpAngle / 180.f));
            
            // en baixada 8angle + de 90 graus) 
            // comprovem si arriba a terra abans d'arribar a 180 graus
            if(jumpAngle > 90)
            {
                if(map->collisionMoveDown(hitboxPos(), glm::ivec2(HITBOX_WIDTH, HITBOX_HEIGHT), &posPlayer.y))
                {
                    bJumping = false;
                    sprite->changeAnimation(lookingRight ? STAND_RIGHT : STAND_LEFT);
                }
            }
        }
    }
    else
    {
		posPlayer.y += FALL_STEP;
        
        // si collisionMoveDown retorna false, vol dir que no està tocant terra i per tant està caient
        if(!map->collisionMoveDown(hitboxPos(), glm::ivec2(HITBOX_WIDTH, HITBOX_HEIGHT), &posPlayer.y))
        {
            // activar anim de caiguda
            int fallAnim = lookingRight ? JUMP_FALL_RIGHT : JUMP_FALL_LEFT;
            if(sprite->animation() != fallAnim)
                sprite->changeAnimation(fallAnim);
        }
        else // si toca terra
        {
            if(Game::instance().getKey(GLFW_KEY_UP))
            {
                bJumping = true;
                jumpAngle = 0;
                startY = posPlayer.y;
                
                // activar anim de pujada
                sprite->changeAnimation(lookingRight ? JUMP_UP_RIGHT : JUMP_UP_LEFT);
            }
            else
            {
                // si acaba de caure, canviar animació a STAND
                if(sprite->animation() == JUMP_FALL_LEFT || sprite->animation() == JUMP_FALL_RIGHT ||
                   sprite->animation() == JUMP_UP_LEFT || sprite->animation() == JUMP_UP_RIGHT)
                {
                    sprite->changeAnimation(lookingRight ? STAND_RIGHT : STAND_LEFT);
                }
            }
        }
    }
    
    sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
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




