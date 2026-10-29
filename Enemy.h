#ifndef _ENEMY_INCLUDE
#define _ENEMY_INCLUDE


#include "Sprite.h"
#include "TileMap.h"


// Enemy es un enemigo del nivel. Hay 7 tipos (una fila de images/enemies.png
// cada uno) y cada tipo se mueve de una de estas tres formas:
//   WALKER: anda a izquierda y derecha y se da la vuelta en paredes y bordes
//   JUMPER: como el WALKER, pero cada cierto tiempo da un saltito
//   FLYER:  vuela en horizontal ondulando, sin gravedad
// Si toca al jugador lo mata; si le alcanza una explosion, desaparece.


enum EnemyBehaviour
{
	WALKER, JUMPER, FLYER
};


class Enemy
{
public:
	Enemy();
	~Enemy();

	void init(int type, const glm::ivec2 &tileMapPos, Texture *spritesheet, ShaderProgram &shaderProgram);
	void update(int deltaTime);
	void render();

	void setTileMap(TileMap *tileMap) { map = tileMap; }
	void setPosition(const glm::ivec2 &pos);
	void setDirection(int dir);

	glm::ivec2 hitboxPos() const;
	glm::ivec2 hitboxSize() const;

	void kill();
	bool isDying() const { return bDying; }
	bool isGone() const { return bDying && dyingTime <= 0; }

private:
	void updateWalk();
	void updateJump();
	void updateFly();
	bool groundAhead() const;
	void turnAround();

private:
	EnemyBehaviour behaviour;
	glm::ivec2 tileMapDispl, posEnemy;
	int direction;      // -1 izquierda, +1 derecha
	float velY;         // velocidad vertical de los saltos
	bool bOnGround;
	int jumpTimer;
	float flyTime;
	int flyBaseY;
	bool bDying;
	int dyingTime;
	Sprite *sprite;
	TileMap *map;

};


#endif // _ENEMY_INCLUDE
