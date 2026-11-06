#ifndef _HUD_INCLUDE
#define _HUD_INCLUDE


#include <glm/glm.hpp>
#include "Texture.h"
#include "Sprite.h"
#include "Text.h"



#define HUD_WIDTH 320
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
	void reset();
	void update(int deltaTime, const HudState &state);
	void render(const HudState &state);

private:
	void renderCell(char c, int cellX, const glm::vec4 &color);
	void renderDigit(int value, int cellX, int blink);
	glm::vec4 blinkColor(int blink) const;

private:
	Texture hudSheet, heartSheet;
	Sprite *hudSprite, *heartSprite;
	Text text;
	enum { BLINK_HEART, BLINK_LIVES, BLINK_ENEMIES, BLINK_BOMBS, BLINK_BLAST, NUM_BLINKS };
	int blinkTime[NUM_BLINKS];
	HudState last;
	bool bHasLast;
	int clock;

};


#endif
