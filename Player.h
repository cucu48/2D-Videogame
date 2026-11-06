#ifndef _PLAYER_INCLUDE
#define _PLAYER_INCLUDE


#include "Sprite.h"
#include "TileMap.h"
#include <string>



class Player
{
public:
	Player();
	~Player();

public:
	void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram);
	void update(int deltaTime);
	void render();
	
	void setTileMap(TileMap *tileMap);
	void setPosition(const glm::vec2 &pos);

	glm::ivec2 hitboxPos() const;
	glm::ivec2 hitboxSize() const;

	void kill();
	void respawn(const glm::vec2 &pos);
	bool isDead() const { return bDead; }
	bool isInvulnerable() const { return invulnerableTime > 0; }
	void startInvulnerable();
	void startInvulnerable(int ms);

	void toggleDance();
	void stopDance();
	bool isDancing() const { return bDancing; }

	bool hasHeart() const { return bHeart; }
	void setHeart(bool heart) { bHeart = heart; }

	float getSpeedMul() const { return speedMul; }
	void setSpeedMul(float mul) { speedMul = mul; }
	void addSpeed();

	bool hasWings() const { return bGod || bWings; }
	bool hasRealWings() const { return bWings; }
	void setWings(bool wings) { bWings = wings; }

	void setGodMode(bool god) { bGod = god; }
	
private:
	bool nudgeHorizontal();
	bool nudgeVertical();
	void startJump();
	bool lookingRight;
	bool bJumping;
	bool bRising;
	bool bOnGround;
	bool bDead;
	bool bHeart;
	bool bWings;
	bool bGod;
	bool upWasPressed;
	bool leftWasPressed, rightWasPressed;
	int lastDir;
	int curJumpHeight;
	float speedMul;
	bool bDancing;
	float moveAccum;
	int invulnerableTime;
	int jumpCooldown;
	int coyoteTime;
	int jumpBuffer;
	glm::ivec2 tileMapDispl, posPlayer;
	int jumpAngle, startY;
	Texture spritesheet;
	Sprite *sprite;
	Texture danceSheet;
	Sprite *danceSprite;
	TileMap *map;

};


#endif


