#ifndef _ENEMY_INCLUDE
#define _ENEMY_INCLUDE


#include "Sprite.h"
#include "TileMap.h"


// Enemy es un enemigo normal del nivel. El tipo es el numero que se pone en
// levels/Stage1-<n>_enemies.txt y decide el sprite y el comportamiento:
//   0 JACKY:          anda despacio, se para cada 2 tiles; en bordes, paredes y
//                     bombas se para y se da la vuelta
//   1 EYTI:           anda despacio; si ve al jugador delante, en su misma fila
//                     y sin nada en medio, se para un momento y embiste hacia el
//   2, 3, 4 JUMPER:   anda y cada cierto tiempo da un saltito
//   5 CLOUDY:         vuela en horizontal ondulando, se da la vuelta en paredes
//                     y bombas, y dispara burbujas si el jugador esta a 4
//                     tiles o menos
//   6 SPRINGCHICKEN:  salta un tile hacia arriba, avanza un tile y cae; se da
//                     la vuelta al chocar con una pared o una bomba, o al
//                     llegar al borde del suelo donde esta
//   7 ELEKIN MAN:     flota a izquierda y derecha; si el jugador esta justo
//                     debajo, se para y suelta un rayo de 2 tiles durante 3 s
//   8 LEAF:           la hoja que invoca Trent: persigue al jugador
//                     atravesando paredes y desaparece al cabo de un rato
// Todos hacen dano al tocarlos y mueren con una explosion.


enum EnemyType
{
	JACKY, EYTI, GREEN, ORANGE, BLACK, CLOUDY, SPRINGCHICKEN, ELEKIN, LEAF, NUM_ENEMY_TYPES
};


class Enemy
{
public:
	Enemy();
	~Enemy();

	void init(int type, const glm::ivec2 &tileMapPos, Texture *spritesheet, ShaderProgram &shaderProgram);
	// playerCenter: centro del jugador en pixeles del mapa (para los que le buscan)
	void update(int deltaTime, const glm::ivec2 &playerCenter, bool playerAlive);
	void render();

	void setTileMap(TileMap *tileMap) { map = tileMap; }
	void setPosition(const glm::ivec2 &pos);
	void setDirection(int dir);
	glm::ivec2 spriteSize() const { return size; }

	glm::ivec2 hitboxPos() const;
	glm::ivec2 hitboxSize() const;

	// Rayo de Elekin Man: true si esta activo, y su caja en pos/size
	bool attackBox(glm::ivec2 &pos, glm::ivec2 &boxSize) const;
	// Cloudy: true si acaba de disparar una burbuja (posicion de salida y direccion)
	bool takeShot(glm::vec2 &pos, glm::vec2 &dir);

	void kill();
	bool isDying() const { return bDying; }
	bool isGone() const { return bDying && dyingTime <= 0; }

private:
	void updateJacky(int deltaTime);
	void updateEyti(int deltaTime, const glm::ivec2 &playerCenter, bool playerAlive);
	void updateJumper(int deltaTime);
	void updateCloudy(int deltaTime, const glm::ivec2 &playerCenter, bool playerAlive);
	void updateSpringchicken(int deltaTime);
	void updateElekin(int deltaTime, const glm::ivec2 &playerCenter, bool playerAlive);
	void updateLeaf(int deltaTime, const glm::ivec2 &playerCenter);

	bool fall();                       // gravedad; devuelve true si esta en el suelo
	int step(float speed);             // pixeles a mover este frame (admite velocidades con decimales)
	bool moveX(int dx);                // mueve en horizontal; false (y no se mueve) si choca
	bool groundAhead() const;          // hay suelo justo delante de los pies
	bool seesPlayer(const glm::ivec2 &playerCenter) const; // misma fila y nada solido en medio
	void turnAround();
	void updateAnimation();

private:
	int type;
	glm::ivec2 tileMapDispl, posEnemy, size;
	int direction;      // -1 izquierda, +1 derecha
	float velY;
	float subPixel;     // parte fraccionaria de la velocidad pendiente de aplicar
	bool bOnGround;
	int state, stateTime, timer;
	int groundY;        // Springchicken: y del suelo sobre el que salta (no se sale de el)
	int walked;         // pixeles andados desde la ultima pausa (Jacky) o en el salto (Springchicken)
	float flyTime;      // Cloudy: para ondular
	int flyBaseY;
	int shotCooldown;
	bool bShot;
	glm::vec2 shotDir;
	glm::vec2 leafPos;
	int lifeTime;
	bool bDying;
	int dyingTime;
	Sprite *sprite, *boltSprite;
	TileMap *map;

};


#endif // _ENEMY_INCLUDE
