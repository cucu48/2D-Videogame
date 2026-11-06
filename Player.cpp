#include <cmath>
#include <algorithm>
#include <iostream>
#include <GL/glew.h>
#include "Player.h"
#include "Game.h"
#include "Audio.h"
#include "Sounds.h"


#define JUMP_ANGLE_STEP 4
#define JUMP_HEIGHT 96
#define FLAP_HEIGHT 40
#define MAX_SPEED_MUL 2.f
#define SPEED_STEP 0.2f
#define FALL_STEP 4
#define MOVE_SPEED 2.5f
#define HITBOX_OFFSET_X 4
#define HITBOX_WIDTH 24
#define HITBOX_HEIGHT 32
#define INVULNERABLE_TIME 2000
#define DANCE_MUSIC "sounds/default_dance.mp3"
#define JUMP_COOLDOWN 450
#define COYOTE_TIME 100
#define JUMP_BUFFER 150
#define CORNER_TOLERANCE 8


enum PlayerAnims
{
	STAND_LEFT, STAND_RIGHT, MOVE_LEFT, MOVE_RIGHT, 
    JUMP_UP_LEFT, JUMP_FALL_LEFT, JUMP_UP_RIGHT, JUMP_FALL_RIGHT, DEATH

};


Player::Player()
{
	sprite = NULL;
	danceSprite = NULL;
	map = NULL;
}

Player::~Player()
{
	stopDance();
	if (sprite != NULL)
		delete sprite;
	if (danceSprite != NULL)
		delete danceSprite;
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
	bHeart = false;
	bWings = false;
	bGod = false;
	upWasPressed = false;
	leftWasPressed = rightWasPressed = false;
	lastDir = 1;
	curJumpHeight = JUMP_HEIGHT;
	speedMul = 1.f;
	bDancing = false;
	moveAccum = 0.f;
	invulnerableTime = 0;
	jumpCooldown = 0;
	coyoteTime = 0;
	jumpBuffer = 0;
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

    	sprite->setAnimationSpeed(JUMP_UP_LEFT, 4);
    	sprite->addKeyframe(JUMP_UP_LEFT, glm::vec2(0.f, 0.5f));
    
    	sprite->setAnimationSpeed(JUMP_FALL_LEFT, 4);
    	sprite->addKeyframe(JUMP_FALL_LEFT, glm::vec2(0.25f, 0.5f));

    	sprite->setAnimationSpeed(JUMP_UP_RIGHT, 4);
    	sprite->addKeyframe(JUMP_UP_RIGHT, glm::vec2(0.5f, 0.5f));
    
    	sprite->setAnimationSpeed(JUMP_FALL_RIGHT, 4);
    	sprite->addKeyframe(JUMP_FALL_RIGHT, glm::vec2(0.75f, 0.5f));

    	sprite->setAnimationSpeed(DEATH, 8);
    	sprite->addKeyframe(DEATH, glm::vec2(0.75f, 0.f));
    	sprite->addKeyframe(DEATH, glm::vec2(0.75f, 0.25f));
		
	sprite->changeAnimation(0);

	danceSheet.loadFromFile("images/player_dance.png", TEXTURE_PIXEL_FORMAT_RGBA);
	danceSheet.setMinFilter(GL_NEAREST);
	danceSheet.setMagFilter(GL_NEAREST);
	danceSprite = Sprite::createSprite(glm::ivec2(32, 32), glm::vec2(0.25f, 0.5f), &danceSheet, &shaderProgram);
	danceSprite->setNumberAnimations(1);
	danceSprite->setAnimationSpeed(0, 7);
	const int danceSteps[16] = { 0, 1, 0, 1, 2, 3, 2, 3, 4, 5, 4, 5, 6, 7, 6, 7 };
	for(int i = 0; i < 16; i++)
		danceSprite->addKeyframe(0, glm::vec2(0.25f * (danceSteps[i] % 4), 0.5f * (danceSteps[i] / 4)));
	danceSprite->changeAnimation(0);

	tileMapDispl = tileMapPos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
	
}

void Player::update(int deltaTime)
{
    sprite->update(deltaTime);
    if(invulnerableTime > 0)
        invulnerableTime -= deltaTime;
    if(jumpCooldown > 0)
        jumpCooldown -= deltaTime;
    if(coyoteTime > 0)
        coyoteTime -= deltaTime;
    if(jumpBuffer > 0)
        jumpBuffer -= deltaTime;

    if(bDead)
        return;

    if(bDancing)
    {
        if(Game::instance().getKey(GLFW_KEY_LEFT) || Game::instance().getKey(GLFW_KEY_RIGHT) ||
           Game::instance().getKey(GLFW_KEY_UP))
            stopDance();
        else
        {
            danceSprite->update(deltaTime);
            return;
        }
    }

    glm::ivec2 hbSize(HITBOX_WIDTH, HITBOX_HEIGHT);
    bool bInAir = bJumping || !bOnGround;
    moveAccum += MOVE_SPEED * getSpeedMul();
    int speed = int(moveAccum);
    moveAccum -= speed;
    
    bool leftNow = Game::instance().getKey(GLFW_KEY_LEFT);
    bool rightNow = Game::instance().getKey(GLFW_KEY_RIGHT);
    if(leftNow && !leftWasPressed)
        lastDir = -1;
    if(rightNow && !rightWasPressed)
        lastDir = 1;
    leftWasPressed = leftNow;
    rightWasPressed = rightNow;
    int dir = 0;
    if(leftNow && rightNow)
        dir = lastDir;
    else if(leftNow)
        dir = -1;
    else if(rightNow)
        dir = 1;
    if(dir < 0)
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
    else if(dir > 0)
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
    
    bool upNow = Game::instance().getKey(GLFW_KEY_UP);
    if(upNow && !upWasPressed)
        jumpBuffer = JUMP_BUFFER;
    if(upNow && !bJumping && !bOnGround && coyoteTime > 0 && jumpCooldown <= 0)
    {
        startJump();
        upWasPressed = upNow;
    }
    if(hasWings() && upNow && !upWasPressed && !bOnGround)
    {
        bJumping = true;
        bRising = true;
        jumpAngle = 0;
        startY = posPlayer.y;
        curJumpHeight = FLAP_HEIGHT;
        Audio::instance().playSound(SFX_JUMP);
    }
    upWasPressed = upNow;

    if(bRising)
    {
        jumpAngle += JUMP_ANGLE_STEP;
        if(jumpAngle >= 90)
        {
            posPlayer.y = startY - curJumpHeight;
            bRising = false;
        }
        else
            posPlayer.y = int(startY - curJumpHeight * sin(3.14159f * jumpAngle / 180.f));

        int freeY = posPlayer.y;
        if(map->collisionMoveUp(hitboxPos(), hbSize, &posPlayer.y))
        {
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
        posPlayer.y += FALL_STEP;
        bOnGround = map->collisionMoveDown(hitboxPos(), hbSize, &posPlayer.y);

        if(bOnGround)
        {
            bJumping = false;
            coyoteTime = COYOTE_TIME;
            if((Game::instance().getKey(GLFW_KEY_UP) || jumpBuffer > 0) && jumpCooldown <= 0)
                startJump();
        }
    }

    int targetAnim = -1;
    if(bRising)
        targetAnim = lookingRight ? JUMP_UP_RIGHT : JUMP_UP_LEFT;
    else if(!bOnGround)
        targetAnim = lookingRight ? JUMP_FALL_RIGHT : JUMP_FALL_LEFT;
    else if(sprite->animation() == JUMP_FALL_LEFT || sprite->animation() == JUMP_FALL_RIGHT ||
            sprite->animation() == JUMP_UP_LEFT || sprite->animation() == JUMP_UP_RIGHT)
        targetAnim = lookingRight ? STAND_RIGHT : STAND_LEFT;
    
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

bool Player::nudgeVertical()
{
    glm::ivec2 hbSize(HITBOX_WIDTH, HITBOX_HEIGHT);
    for(int d = 1; d <= CORNER_TOLERANCE; d++)
        for(int s = -1; s <= 1; s += 2)
            if(!map->collisionBox(hitboxPos() + glm::ivec2(0, s * d), hbSize))
            {
                posPlayer.y += s * d;
                startY += s * d;
                return true;
            }
    return false;
}

void Player::toggleDance()
{
    if(bDancing)
    {
        stopDance();
        return;
    }
    if(bDead || !bOnGround)
        return;
    bDancing = true;
    danceSprite->changeAnimation(0);
    danceSprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
    Audio::instance().pushMusic(DANCE_MUSIC, true);
}

void Player::stopDance()
{
    if(!bDancing)
        return;
    bDancing = false;
    Audio::instance().popMusic();
    sprite->changeAnimation(lookingRight ? STAND_RIGHT : STAND_LEFT);
}

void Player::startJump()
{
    jumpCooldown = JUMP_COOLDOWN;
    coyoteTime = 0;
    jumpBuffer = 0;
    bJumping = true;
    Audio::instance().playSound(SFX_JUMP);
    curJumpHeight = JUMP_HEIGHT;
    bRising = true;
    bOnGround = false;
    jumpAngle = 0;
    startY = posPlayer.y;
}

void Player::kill()
{
    if(bDead)
        return;
    stopDance();
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
    startInvulnerable();
    sprite->changeAnimation(STAND_RIGHT);
    setPosition(pos);
}

void Player::startInvulnerable()
{
	startInvulnerable(INVULNERABLE_TIME);
}

void Player::startInvulnerable(int ms)
{
	if(ms > invulnerableTime)
		invulnerableTime = ms;
}

void Player::addSpeed()
{
	speedMul = std::min(speedMul + SPEED_STEP, MAX_SPEED_MUL);
	if(speedMul > MAX_SPEED_MUL - 0.01f)
		speedMul = MAX_SPEED_MUL;
}

void Player::render()
{
	if(invulnerableTime > 0 && (invulnerableTime / 100) % 2 == 0)
		return;
	if(bDancing)
		danceSprite->render();
	else
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




