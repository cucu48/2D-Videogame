#ifndef _PLAYER_INCLUDE
#define _PLAYER_INCLUDE


#include "Sprite.h"
#include "TileMap.h"
#include <string>


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
	void startInvulnerable();   // blinking immunity, also used after respawning
	void startInvulnerable(int ms); // idem for ms milliseconds (armored jacket); never shortens a longer one

	// Baile (tecla B): solo en el suelo; se corta al moverse, saltar o morir
	void toggleDance();
	void stopDance();
	bool isDancing() const { return bDancing; }

	// Heart power-up: absorbs the next hit
	bool hasHeart() const { return bHeart; }
	void setHeart(bool heart) { bHeart = heart; }

	// Speed Up power-up: 1x up to 2x, +0.2x each
	float getSpeedMul() const { return speedMul; }
	void setSpeedMul(float mul) { speedMul = mul; }
	void addSpeed();

	// Wings power-up: pressing UP in the air flaps, no floor needed
	bool hasWings() const { return bWings; }
	void setWings(bool wings) { bWings = wings; }
	
private:
	bool nudgeHorizontal();
	bool nudgeVertical();
	bool lookingRight;
	bool bJumping;   // desde que despega hasta que vuelve a tocar suelo
	bool bRising;    // solo la parte de subida del salto
	bool bOnGround;
	bool bDead;
	bool bHeart;
	bool bWings;
	bool upWasPressed;    // UP key last frame, to detect a new press (flap)
	int curJumpHeight;    // JUMP_HEIGHT on the ground, FLAP_HEIGHT when flapping
	float speedMul;
	bool bDancing;
	std::string musicBeforeDance;   // se vuelve a poner al acabar el baile
	bool musicBeforeDanceLoops;
	float moveAccum;      // parte fraccionaria de MOVE_SPEED pendiente de aplicar
	int invulnerableTime; // tras reaparecer, un rato sin poder morir (parpadea)
	glm::ivec2 tileMapDispl, posPlayer;
	int jumpAngle, startY;
	Texture spritesheet;
	Sprite *sprite;
	Texture danceSheet;
	Sprite *danceSprite;   // el baile usa otra hoja de sprites (images/player_dance.png)
	TileMap *map;

};


#endif // _PLAYER_INCLUDE


