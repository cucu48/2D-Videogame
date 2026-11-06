#include <cmath>
#include <algorithm>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include "SecretScene.h"
#include "Game.h"
#include "Audio.h"
#include "Sounds.h"


#define VIRTUAL_WIDTH 320
#define VIRTUAL_HEIGHT 288
#define FX_W 128
#define FX_H 32
#define FADE_IN_TIME 1600
#define FADE_OUT_TIME 1200
#define PROMPT_TIME 6000
#define SKIP_TIME 2500
#define BLINK_TIME 500

#define CREDITS_START 2000
#define CREDITS_SPEED 0.016f
#define CREDITS_BOTTOM 160.f
#define CREDITS_FADE_FROM 52.f
#define CREDITS_FADE_TO 8.f
#define CREDITS_LAST_Y 72.f

#define SMOKE_LIFE 2400
#define SMOKE_EMIT 170

#define PUFF_PERIOD 4200
#define INHALE_TIME 1500
#define EXHALE_START 2100
#define EXHALE_TIME 1300


struct Smoker
{
	float x, y;
	int scale;
	int period, offset;
	int exhaleStart, exhaleTime;
	float dir;
};

static const Smoker SMOKERS[] = {
	{ 114.f, 208.f, 3, PUFF_PERIOD, 0,               EXHALE_START, EXHALE_TIME, 1.f },
	{ 228.f, 212.f, 3, PUFF_PERIOD, PUFF_PERIOD / 2, EXHALE_START, EXHALE_TIME, -1.f },
};
#define NUM_SMOKERS int(sizeof(SMOKERS) / sizeof(SMOKERS[0]))




static TexturedQuad *makeQuad(ShaderProgram &program, float w, float h, glm::vec2 uv0, glm::vec2 uv1)
{
	glm::vec2 geom[2] = { glm::vec2(0.f, 0.f), glm::vec2(w, h) };
	glm::vec2 texCoords[2] = { uv0, uv1 };
	return TexturedQuad::createTexturedQuad(geom, texCoords, program);
}

static TexturedQuad *makeCell(ShaderProgram &program, int col, int row)
{
	glm::vec2 uv0(col * 16.f / FX_W, row * 16.f / FX_H);
	glm::vec2 uv1((col + 1) * 16.f / FX_W, (row + 1) * 16.f / FX_H);
	return makeQuad(program, 16.f, 16.f, uv0, uv1);
}

static void loadTex(Texture &tex, const char *file)
{
	tex.loadFromFile(file, TEXTURE_PIXEL_FORMAT_RGBA);
	tex.setMinFilter(GL_NEAREST);
	tex.setMagFilter(GL_NEAREST);
}


SecretScene::SecretScene()
{
	bgQuad = fgQuad = promptQuad = NULL;
	for(int i = 0; i < 2; i++)
		for(int j = 0; j < 3; j++)
			bongQuads[i][j] = NULL;
	for(int i = 0; i < 4; i++)
		smokeQuads[i] = NULL;
	time = 0;
	leaveTime = -1;
	bInScene = false;
}

SecretScene::~SecretScene()
{
	TexturedQuad *quads[] = { bgQuad, fgQuad, promptQuad, bongQuads[0][0], bongQuads[0][1], bongQuads[0][2],
	                          bongQuads[1][0], bongQuads[1][1], bongQuads[1][2],
	                          smokeQuads[0], smokeQuads[1], smokeQuads[2], smokeQuads[3] };
	for(size_t i = 0; i < sizeof(quads) / sizeof(quads[0]); i++)
		if(quads[i] != NULL)
		{
			quads[i]->free();
			delete quads[i];
		}
}

void SecretScene::init()
{
	ShaderProgram &program = Game::instance().getTexProgram();
	loadTex(bgTex, "images/secret_bg.png");
	loadTex(fgTex, "images/secret_fg.png");
	loadTex(fxTex, "images/secret_fx.png");
	loadTex(promptTex, "images/PressSpace.png");
	bgQuad = makeQuad(program, VIRTUAL_WIDTH, VIRTUAL_HEIGHT, glm::vec2(0.f), glm::vec2(1.f));
	fgQuad = makeQuad(program, VIRTUAL_WIDTH, VIRTUAL_HEIGHT, glm::vec2(0.f), glm::vec2(1.f));
	credits.init();
	promptQuad = makeQuad(program, float(promptTex.width()), float(promptTex.height()), glm::vec2(0.f), glm::vec2(1.f));
	for(int row = 0; row < 2; row++)
		for(int f = 0; f < 3; f++)
			bongQuads[row][f] = makeCell(program, f, row);
	for(int i = 0; i < 4; i++)
		smokeQuads[i] = makeCell(program, 3 + i, 0);
	fade.init(program, VIRTUAL_WIDTH, VIRTUAL_HEIGHT);
}

void SecretScene::onEnter()
{
	time = 0;
	leaveTime = -1;
	bInScene = true;
}

void SecretScene::update(int deltaTime)
{
	time += deltaTime;
	if(leaveTime >= 0)
	{
		leaveTime += deltaTime;
		if(leaveTime >= FADE_OUT_TIME && bInScene)
		{
			bInScene = false;
			Game::instance().changeScene(SCENE_MENU);
		}
	}
}

void SecretScene::keyPressed(int key)
{
	if((key == GLFW_KEY_SPACE || key == GLFW_KEY_ENTER) && leaveTime < 0 && time >= SKIP_TIME)
	{
		leaveTime = 0;
		Audio::instance().playSound(SFX_SELECT);
	}
}

void SecretScene::renderCell(TexturedQuad *quad, const glm::vec2 &pos, float scale)
{
	ShaderProgram &program = Game::instance().getTexProgram();
	glm::mat4 m = glm::translate(glm::mat4(1.f), glm::vec3(glm::floor(pos), 0.f));
	m = glm::scale(m, glm::vec3(scale, scale, 1.f));
	program.setUniformMatrix4f("modelview", m);
	quad->render(fxTex);
}

void SecretScene::renderCredits(const glm::mat4 &projection)
{
	CreditsLayout layout;
	layout.width = VIRTUAL_WIDTH;
	layout.bottom = CREDITS_BOTTOM;
	layout.fadeFrom = CREDITS_FADE_FROM;
	layout.fadeTo = CREDITS_FADE_TO;
	layout.lastY = CREDITS_LAST_Y;
	layout.speed = CREDITS_SPEED;
	credits.render(projection, time - CREDITS_START, layout);
}

void SecretScene::render(int windowWidth, int windowHeight)
{
	glm::mat4 projection = setLetterboxViewport(windowWidth, windowHeight, VIRTUAL_WIDTH, VIRTUAL_HEIGHT);
	ShaderProgram &program = Game::instance().getTexProgram();
	program.use();
	program.setUniformMatrix4f("projection", projection);
	program.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
	program.setUniform2f("texCoordDispl", 0.f, 0.f);
	glm::mat4 modelview(1.f);
	program.setUniformMatrix4f("modelview", modelview);
	bgQuad->render(bgTex);

	renderCredits(projection);
	program.use();
	program.setUniformMatrix4f("projection", projection);
	program.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
	program.setUniform2f("texCoordDispl", 0.f, 0.f);
	program.setUniformMatrix4f("modelview", modelview);
	fgQuad->render(fgTex);

	int frame = 0;
	for(int i = 0; i < 2; i++)
	{
		int c = (time + SMOKERS[i].offset) % PUFF_PERIOD;
		if(c < INHALE_TIME)
			frame = 1 + (c / 160) % 2;
	}
	renderCell(bongQuads[0][frame], glm::vec2(146.f, 202.f), 4.f);

	for(int s = 0; s < NUM_SMOKERS; s++)
	{
		const Smoker &sm = SMOKERS[s];
		int first = std::max(0, (time - SMOKE_LIFE) / SMOKE_EMIT + 1) * SMOKE_EMIT;
		for(int t0 = first; t0 <= time; t0 += SMOKE_EMIT)
		{
			int c = (t0 + sm.offset) % sm.period;
			if(c < sm.exhaleStart || c >= sm.exhaleStart + sm.exhaleTime)
				continue;
			float age = float(time - t0);
			int cell = age < 300.f ? 0 : age < 800.f ? 1 : age < 1500.f ? 2 : 3;
			float k = sm.scale / 3.f;
			float sec = age / 1000.f;
			glm::vec2 p(sm.x + sm.dir * k * (10.f * (1.f - std::exp(-sec * 3.f)) + 4.f * sec)
			                 + k * 3.f * std::sin(sec * 2.6f + t0 * 0.37f),
			            sm.y - k * 16.f * sec - k * 2.f * sec * sec);
			float half = 8.f * sm.scale;
			renderCell(smokeQuads[cell], p - glm::vec2(half), float(sm.scale));
		}
	}

	program.setUniformMatrix4f("modelview", modelview);
	if(time >= PROMPT_TIME && leaveTime < 0 && ((time - PROMPT_TIME) / BLINK_TIME) % 2 == 0)
	{
		glm::mat4 m = glm::translate(glm::mat4(1.f), glm::vec3((VIRTUAL_WIDTH - promptTex.width()) / 2, VIRTUAL_HEIGHT - 14, 0.f));
		program.setUniformMatrix4f("modelview", m);
		promptQuad->render(promptTex);
	}

	int level = 0;
	if(time < FADE_IN_TIME)
		level = FADE_LEVELS - time * (FADE_LEVELS + 1) / FADE_IN_TIME;
	if(leaveTime >= 0)
		level = std::max(level, (leaveTime * (FADE_LEVELS + 1)) / FADE_OUT_TIME);
	fade.render(program, level);
}
