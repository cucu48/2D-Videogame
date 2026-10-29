#ifndef _PLAYER_INCLUDE
#define _PLAYER_INCLUDE


#include "Sprite.h"
#include "TileMap.h"


// Player is basically a Sprite that represents the player. As such it has
// all properties it needs to track its movement, jumping, and collisions.


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
	
private:
	bool nudgeHorizontal();
	bool nudgeVertical();
	bool lookingRight;
	bool bJumping;   // desde que despega hasta que vuelve a tocar suelo
	bool bRising;    // solo la parte de subida del salto
	bool bOnGround;
	bool bDead;
	int invulnerableTime; // tras reaparecer, un rato sin poder morir (parpadea)
	glm::ivec2 tileMapDispl, posPlayer;
	int jumpAngle, startY;
	Texture spritesheet;
	Sprite *sprite;
	TileMap *map;

};


#endif // _PLAYER_INCLUDE


