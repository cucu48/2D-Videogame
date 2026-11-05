#include <cstdio>
#include <iostream>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "Hud.h"
#include "Game.h"


#define TILE 16                 // one 8x8 tile of HUD.png at 2x
#define TEXT_SIZE 12            // glyph height in virtual pixels
#define TEXT_COLOR glm::vec4(0.f, 0.f, 0.f, 1.f)
#define BLINK_COLOR glm::vec4(0.9f, 0.05f, 0.05f, 1.f)
#define BLINK_TIME 900          // ms que parpadea un valor al cambiar
#define BLINK_PERIOD 150
#define HURRY_SECONDS 30        // a partir de aqui el tiempo sale en rojo (y en los 10 ultimos parpadea)

// Tiles of the bar (0-based) where each value goes
#define TIME_TILE 2
#define HEART_TILE 7
#define LIVES_TILE 9
#define ENEMIES_TILE 12
#define BOMBS_TILE 15
#define BLAST_TILE 18


Hud::Hud()
{
	hudSprite = NULL;
	heartSprite = NULL;
	reset();
}

void Hud::reset()
{
	for(int i = 0; i < NUM_BLINKS; i++)
		blinkTime[i] = 0;
	bHasLast = false;
	clock = 0;
}

void Hud::update(int deltaTime, const HudState &state)
{
	clock += deltaTime;
	for(int i = 0; i < NUM_BLINKS; i++)
		blinkTime[i] = std::max(0, blinkTime[i] - deltaTime);
	if(bHasLast)
	{
		if(state.hasHeart != last.hasHeart) blinkTime[BLINK_HEART] = BLINK_TIME;
		if(state.lives != last.lives) blinkTime[BLINK_LIVES] = BLINK_TIME;
		if(state.enemiesLeft != last.enemiesLeft) blinkTime[BLINK_ENEMIES] = BLINK_TIME;
		if(state.maxBombs != last.maxBombs) blinkTime[BLINK_BOMBS] = BLINK_TIME;
		if(state.blastRange != last.blastRange) blinkTime[BLINK_BLAST] = BLINK_TIME;
	}
	last = state;
	bHasLast = true;
}

// Mientras parpadea, alterna entre rojo y el color normal
glm::vec4 Hud::blinkColor(int blink) const
{
	if(blink > 0 && (blink / BLINK_PERIOD) % 2 == 0)
		return BLINK_COLOR;
	return TEXT_COLOR;
}

Hud::~Hud()
{
	if(hudSprite != NULL)
		delete hudSprite;
	if(heartSprite != NULL)
		delete heartSprite;
}

void Hud::init()
{
	ShaderProgram &texProgram = Game::instance().getTexProgram();

	hudSheet.loadFromFile("images/HUD.png", TEXTURE_PIXEL_FORMAT_RGBA);
	hudSheet.setMinFilter(GL_NEAREST);
	hudSheet.setMagFilter(GL_NEAREST);
	heartSheet.loadFromFile("images/HeartHUD.png", TEXTURE_PIXEL_FORMAT_RGBA);
	heartSheet.setMinFilter(GL_NEAREST);
	heartSheet.setMagFilter(GL_NEAREST);
	hudSprite = Sprite::createSprite(glm::vec2(HUD_WIDTH, HUD_HEIGHT), glm::vec2(1.f, 1.f), &hudSheet, &texProgram);
	hudSprite->setPosition(glm::vec2(0.f, 0.f));
	heartSprite = Sprite::createSprite(glm::vec2(TILE, TILE), glm::vec2(1.f, 1.f), &heartSheet, &texProgram);
	heartSprite->setPosition(glm::vec2(float(HEART_TILE * TILE), 0.f));

	if(!text.init("images/bytebounce.medium.ttf"))
		cout << "Could not load images/bytebounce.medium.ttf" << endl;
}

void Hud::render(const HudState &state)
{
	glm::mat4 projection = glm::ortho(0.f, float(HUD_WIDTH), float(HUD_HEIGHT), 0.f);

	ShaderProgram &texProgram = Game::instance().getTexProgram();
	texProgram.use();
	texProgram.setUniformMatrix4f("projection", projection);
	texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
	hudSprite->render();
	// el corazon parpadea al cogerlo o al perderlo
	int heartBlink = blinkTime[BLINK_HEART];
	bool heartVisible = state.hasHeart;
	if(heartBlink > 0)
		heartVisible = (heartBlink / BLINK_PERIOD) % 2 == 0;
	if(heartVisible)
		heartSprite->render();

	text.setProjection(projection);

	// M:SS, one glyph per tile so it always fits in 4 tiles
	int seconds = std::max(0, std::min(state.secondsLeft, 9 * 60 + 59));
	char timeStr[16];
	snprintf(timeStr, sizeof(timeStr), "%d:%02d", seconds / 60, seconds % 60);
	// poco tiempo: en rojo, y los 10 ultimos segundos parpadeando
	glm::vec4 timeColor = TEXT_COLOR;
	if(state.secondsLeft <= HURRY_SECONDS)
		timeColor = BLINK_COLOR;
	if(state.secondsLeft <= 10 && (clock / 250) % 2 == 1)
		timeColor = TEXT_COLOR;
	for(int i = 0; i < 4; i++)
		renderCell(timeStr[i], (TIME_TILE + i) * TILE, timeColor);

	renderDigit(state.lives, LIVES_TILE * TILE, blinkTime[BLINK_LIVES]);
	renderDigit(state.enemiesLeft, ENEMIES_TILE * TILE, blinkTime[BLINK_ENEMIES]);
	renderDigit(state.maxBombs, BOMBS_TILE * TILE, blinkTime[BLINK_BOMBS]);
	renderDigit(state.blastRange, BLAST_TILE * TILE, blinkTime[BLINK_BLAST]);
}

void Hud::renderCell(char c, int cellX, const glm::vec4 &color)
{
	int x = cellX + (TILE - text.charAdvance(c, TEXT_SIZE)) / 2;
	// Text draws from the baseline: put it where the bitmap ends up centered vertically in the tile
	int y = (TILE - text.charHeight(c, TEXT_SIZE)) / 2 + text.charAscent(c, TEXT_SIZE);
	text.render(string(1, c), glm::vec2(float(x), float(y)), TEXT_SIZE, color);
}

void Hud::renderDigit(int value, int cellX, int blink)
{
	renderCell(char('0' + std::max(0, std::min(value, 9))), cellX, blinkColor(blink));
}
