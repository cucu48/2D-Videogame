#ifndef _DOOR_INCLUDE
#define _DOOR_INCLUDE


#include "Sprite.h"
#include "TileMap.h"


// The exit door of a level. It is rigid (a blocked tile) while closed and
// walkable once open. Level decides when to open it and when the player reaches it.
class Door
{

public:
	Door();
	~Door();

	// spritesheet = images/Doors.png (two 16x16 frames: closed, open)
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


#endif // _DOOR_INCLUDE
