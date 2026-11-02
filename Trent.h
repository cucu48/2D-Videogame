#ifndef _TRENT_INCLUDE
#define _TRENT_INCLUDE


#include "Sprite.h"
#include "TileMap.h"


// Trent, el jefe del bosque (tipo 9 en levels/Stage1-<n>_enemies.txt).
// Anda de lado a lado por arriba del mapa atravesando paredes y bombas. Si el
// jugador pasa por delante, a veces baja hasta abajo del todo y vuelve a subir.
// Cada pocos segundos se para y lanza un ataque, que Level recoge con
// takeAttack(): hojas que persiguen, esporas de polen o lluvia de bombas.
// Aguanta 5 golpes de explosion y hace dano al tocarle.


enum TrentAttack
{
	TRENT_NO_ATTACK, TRENT_LEAVES, TRENT_POLLEN, TRENT_BOMB_RAIN
};


class Trent
{
public:
	Trent();
	~Trent();

	void init(const glm::ivec2 &tileMapPos, Texture *spritesheet, ShaderProgram &shaderProgram);
	void update(int deltaTime, const glm::ivec2 &playerCenter, bool playerAlive);
	void render();

	void setTileMap(TileMap *tileMap) { map = tileMap; }
	void setPosition(const glm::ivec2 &pos);

	glm::ivec2 hitboxPos() const;
	glm::ivec2 hitboxSize() const;
	glm::ivec2 getPosition() const { return pos; }
	int spriteSize() const { return 64; }

	bool hit();                     // le alcanza una explosion; true si le ha hecho dano
	void kill();                    // muere al instante (tecla K)
	TrentAttack takeAttack();       // ataque que acaba de lanzar (una sola vez)
	bool isDying() const { return bDying; }
	bool isGone() const { return bDying && dyingTime <= 0; }

private:
	int step(float speed);
	void changeAnim(int anim);

private:
	glm::ivec2 tileMapDispl, pos;
	int topY;
	int direction;
	float subPixel;
	int state, stateTime;
	int attackTimer;
	TrentAttack chosenAttack, pendingAttack;
	bool bPlayerWasInFront;
	int hitsLeft;
	int invulnerableTime;
	bool bDying;
	int dyingTime;
	Sprite *sprite;
	TileMap *map;

};


#endif // _TRENT_INCLUDE
