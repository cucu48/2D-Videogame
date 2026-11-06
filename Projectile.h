#ifndef _PROJECTILE_INCLUDE
#define _PROJECTILE_INCLUDE


#include "Sprite.h"
#include "TileMap.h"



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
	void pop();
	bool hasLanded() const { return bLanded; }
	bool isPopping() const { return bPopping; }
	bool isFinished() const { return bFinished; }
	glm::vec2 getPosition() const { return pos; }

private:
	ProjectileKind kind;
	glm::vec2 pos, vel;
	float travelled;
	bool bPopping, bLanded, bFinished;
	int popTime;
	Sprite *sprite;

};


#endif
