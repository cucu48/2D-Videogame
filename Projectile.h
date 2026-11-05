#ifndef _PROJECTILE_INCLUDE
#define _PROJECTILE_INCLUDE


#include "Sprite.h"
#include "TileMap.h"


// Proyectiles de los enemigos:
//   BUBBLE:       burbuja de Cloudy, va hacia el jugador hasta 4 tiles
//   SPORE:        espora de polen de Trent (bola de energia), baja atravesando
//                 cualquier bloque hasta salir del mapa
//   FALLING_BOMB: bomba de la lluvia de bombas de Trent; cae con gravedad y
//                 al tocar el suelo Level la cambia por una Bomb de verdad
// Todos hacen dano al jugador al tocarle (la bomba que cae no).


enum ProjectileKind
{
	BUBBLE, SPORE, FALLING_BOMB
};


class Projectile
{
public:
	Projectile();
	~Projectile();

	void init(ProjectileKind kind, const glm::vec2 &center, const glm::vec2 &velocity, Texture *spritesheet, ShaderProgram &shaderProgram);
	void update(int deltaTime, TileMap *map);
	void render();

	ProjectileKind getKind() const { return kind; }
	glm::ivec2 hitboxPos() const;
	glm::ivec2 hitboxSize() const;
	bool isHarmful() const { return kind != FALLING_BOMB && !bPopping; }
	void pop();                                // se deshace (al tocar al jugador o una pared)
	bool hasLanded() const { return bLanded; } // solo FALLING_BOMB
	bool isPopping() const { return bPopping; }
	bool isFinished() const { return bFinished; }
	glm::vec2 getPosition() const { return pos; }

private:
	ProjectileKind kind;
	glm::vec2 pos, vel;     // pos: esquina superior izquierda del sprite (32x32)
	float travelled;
	bool bPopping, bLanded, bFinished;
	int popTime;
	Sprite *sprite;

};


#endif // _PROJECTILE_INCLUDE
