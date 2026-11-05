#ifndef _HUD_INCLUDE
#define _HUD_INCLUDE


#include <glm/glm.hpp>
#include "Texture.h"
#include "Sprite.h"
#include "Text.h"


// Hud is the Game Boy style status bar (images/HUD.png) drawn below the playfield.
// It holds no game state: Level fills a HudState every frame and Hud draws it.
// Every value is clamped to a single digit (0-9) when drawn.
// Un valor que cambia parpadea un momento, y el tiempo se pone rojo al final.


#define HUD_WIDTH 320   // virtual pixels: HUD.png (160x8) at 2x
#define HUD_HEIGHT 16


struct HudState
{
	int secondsLeft;
	bool hasHeart;
	int lives, enemiesLeft, maxBombs, blastRange;
};


class Hud
{

public:
	Hud();
	~Hud();

	void init();
	void reset();                                       // nivel nuevo: lo primero que llegue no parpadea
	void update(int deltaTime, const HudState &state);  // detecta los valores que cambian
	// Draws the bar in the current viewport, which has to be the HUD_WIDTH x HUD_HEIGHT strip
	void render(const HudState &state);

private:
	void renderCell(char c, int cellX, const glm::vec4 &color);   // c centered in the 16x16 tile that starts at cellX
	void renderDigit(int value, int cellX, int blink);
	glm::vec4 blinkColor(int blink) const;

private:
	Texture hudSheet, heartSheet;
	Sprite *hudSprite, *heartSprite;
	Text text;
	enum { BLINK_HEART, BLINK_LIVES, BLINK_ENEMIES, BLINK_BOMBS, BLINK_BLAST, NUM_BLINKS };
	int blinkTime[NUM_BLINKS];      // ms que le quedan de parpadeo a cada valor
	HudState last;
	bool bHasLast;
	int clock;                      // ms, para el parpadeo del tiempo

};


#endif // _HUD_INCLUDE
