#ifndef _DOOR_INCLUDE
#define _DOOR_INCLUDE


#include "Sprite.h"
#include "TileMap.h"


class Door
{

public:
	Door();
	~Door();

	void init(const glm::ivec2 &tile, TileMap *tileMap, Texture *spritesheet, ShaderProgram &shaderProgram);
	void render();

	void open();
	bool isOpen() const { return bOpen; }
	glm::ivec2 getTile() const { return tilePos; }

private:
	glm::ivec2 tilePos;
	bool bOpen;
	Sprite *sprite;
	TileMap *map;

};


#endif
