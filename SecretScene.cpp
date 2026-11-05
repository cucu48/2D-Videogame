#include <cmath>
#include <algorithm>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include "SecretScene.h"
#include "Game.h"
#include "Audio.h"
#include "Sounds.h"


#define VIRTUAL_WIDTH 320          // tamano de images/secret_bg.png
#define VIRTUAL_HEIGHT 288
#define FX_W 128                   // images/secret_fx.png: celdas de 16x16
#define FX_H 32
#define FADE_IN_TIME 1600          // ms del fundido desde negro
#define FADE_OUT_TIME 1200         // ms del fundido a negro al salir
#define PROMPT_TIME 6000           // ms hasta que sale "PRESS SPACE"
#define SKIP_TIME 2500             // antes de esto no se puede salir (para no saltarla sin querer)
#define BLINK_TIME 500

// Creditos: salen de detras de las colinas y suben hasta desaparecer arriba
#define CREDITS_START 2000         // ms: empiezan cuando ya se ve la escena
#define CREDITS_SPEED 0.016f       // px por ms (unos 16 px/s)
#define CREDITS_BOTTOM 160.f       // y (linea base) donde nace cada linea, tapada por las colinas
#define CREDITS_FADE_FROM 52.f     // por encima de aqui (parte de arriba del texto) se van apagando...
#define CREDITS_FADE_TO 8.f        // ...y aqui ya no se ven
#define CREDITS_LAST_Y 72.f        // la ultima linea ("GRACIES PER JUGAR") se queda aqui

// Humo: cada bocanada vive SMOKE_LIFE ms, sube y crece (celdas 3..6)
#define SMOKE_LIFE 2400
#define SMOKE_EMIT 170             // ms entre bocanadas mientras alguien exhala

// Bomberman y Eyti: calada (burbujas y brasa) y luego sueltan el humo
#define PUFF_PERIOD 4200
#define INHALE_TIME 1500
#define EXHALE_START 2100
#define EXHALE_TIME 1300


// Alguien que echa humo: boca (centro de la bocanada al salir), escala,
// ciclo, desfase y hacia donde se va el humo
struct Smoker
{
	float x, y;
	int scale;
	int period, offset;
	int exhaleStart, exhaleTime;
	float dir;
};

static const Smoker SMOKERS[] = {
	{ 114.f, 208.f, 3, PUFF_PERIOD, 0,               EXHALE_START, EXHALE_TIME, 1.f },    // Bomberman
	{ 228.f, 212.f, 3, PUFF_PERIOD, PUFF_PERIOD / 2, EXHALE_START, EXHALE_TIME, -1.f },   // Eyti
};
#define NUM_SMOKERS int(sizeof(SMOKERS) / sizeof(SMOKERS[0]))


// Lineas de los creditos, de la primera en salir a la ultima. size 0 = hueco.
// En catalan. La fuente solo tiene ASCII: sin acentos (y la ela geminada con punto).
struct CreditLine
{
	const char *text;
	int size;
	float r, g, b;
};

#define TITLE 16
#define HEAD 12
#define BODY 10
#define GOLD 1.f, 0.85f, 0.35f
#define PINK 1.f, 0.62f, 0.62f
#define CREAM 1.f, 0.95f, 0.85f
static const CreditLine CREDITS[] = {
	{ "POCKET BOMBERMAN", TITLE, GOLD },
	{ "", 0, CREAM },
	{ "UN JOC DE", BODY, CREAM },
	{ "RUBEN GARCIA", HEAD, PINK },
	{ "I PAU CARULLA", HEAD, PINK },
	{ "", 0, CREAM },
	{ "VIDEOJOCS - FIB - UPC", BODY, CREAM },
	{ "", 0, CREAM },
	{ "", 0, CREAM },
	{ "RUBEN GARCIA", HEAD, GOLD },
	{ "FISIQUES I COL.LISIONS", BODY, CREAM },
	{ "BOMBES", BODY, CREAM },
	{ "ENEMICS I BOSS FINAL", BODY, CREAM },
	{ "CASCADES", BODY, CREAM },
	{ "VIDES, TEMPS I GAME OVER", BODY, CREAM },
	{ "MUSICA I SO", BODY, CREAM },
	{ "PANTALLA VS", BODY, CREAM },
	{ "BONGS I FINAL SECRET", BODY, CREAM },
	{ "VERSIO PER A WINDOWS", BODY, CREAM },
	{ "", 0, CREAM },
	{ "", 0, CREAM },
	{ "PAU CARULLA", HEAD, GOLD },
	{ "ESCENES I MENUS", BODY, CREAM },
	{ "MAPES DELS NIVELLS", BODY, CREAM },
	{ "CAMERA I HUD", BODY, CREAM },
	{ "PORTA I ARBUSTOS", BODY, CREAM },
	{ "POWER-UPS", BODY, CREAM },
	{ "REACCIO EN CADENA", BODY, CREAM },
	{ "", 0, CREAM },
	{ "", 0, CREAM },
	{ "BASAT EN", BODY, CREAM },
	{ "POCKET BOMBERMAN", HEAD, GOLD },
	{ "HUDSON SOFT, 1998", BODY, CREAM },
	{ "", 0, CREAM },
	{ "", 0, CREAM },
	{ "", 0, CREAM },
	{ "GRACIES PER JUGAR", TITLE, GOLD },
};
#define NUM_CREDITS int(sizeof(CREDITS) / sizeof(CREDITS[0]))
#define CREDITS_GAP 6              // px entre lineas (los huecos ocupan BODY + CREDITS_GAP)


static TexturedQuad *makeQuad(ShaderProgram &program, float w, float h, glm::vec2 uv0, glm::vec2 uv1)
{
	glm::vec2 geom[2] = { glm::vec2(0.f, 0.f), glm::vec2(w, h) };
	glm::vec2 texCoords[2] = { uv0, uv1 };
	return TexturedQuad::createTexturedQuad(geom, texCoords, program);
}

// Celda (col, row) de 16x16 de images/secret_fx.png
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
	bText = false;
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
	bText = text.init("images/bytebounce.medium.ttf");
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
	if(!bText || time < CREDITS_START)
		return;
	text.setProjection(projection);

	// Distancia de cada linea a la primera: se leen de arriba a abajo y suben
	float scroll = (time - CREDITS_START) * CREDITS_SPEED;
	float offset[NUM_CREDITS];
	float acc = 0.f;
	for(int i = 0; i < NUM_CREDITS; i++)
	{
		offset[i] = acc;
		acc += (CREDITS[i].size > 0 ? CREDITS[i].size : BODY) + CREDITS_GAP;
	}
	// la ultima ("GRACIES PER JUGAR") se para en el cielo y se queda
	scroll = std::min(scroll, CREDITS_BOTTOM + offset[NUM_CREDITS - 1] - CREDITS_LAST_Y);

	for(int i = 0; i < NUM_CREDITS; i++)
	{
		const CreditLine &line = CREDITS[i];
		float base = std::floor(CREDITS_BOTTOM - scroll + offset[i]);
		if(line.size == 0 || base > CREDITS_BOTTOM + line.size || base < 0.f)
			continue;
		// se apagan al llegar arriba (abajo las tapan las colinas)
		float alpha = (base - line.size - CREDITS_FADE_TO) / (CREDITS_FADE_FROM - CREDITS_FADE_TO);
		alpha = std::max(0.f, std::min(1.f, alpha));
		if(alpha <= 0.f)
			continue;
		std::string str(line.text);
		int width = 0;
		for(size_t c = 0; c < str.size(); c++)
			width += text.charAdvance(str[c], line.size);
		glm::vec2 pos(std::floor((VIRTUAL_WIDTH - width) / 2.f), base);
		text.render(str, pos + glm::vec2(1.f, 1.f), line.size, glm::vec4(0.25f, 0.08f, 0.2f, alpha * 0.8f));
		text.render(str, pos, line.size, glm::vec4(line.r, line.g, line.b, alpha));
	}
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

	// Creditos por el cielo, y encima el primer plano (colinas, prado, personajes)
	renderCredits(projection);
	program.use();
	program.setUniformMatrix4f("projection", projection);
	program.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
	program.setUniform2f("texCoordDispl", 0.f, 0.f);
	program.setUniformMatrix4f("modelview", modelview);
	fgQuad->render(fgTex);

	// Bong compartido entre los dos: burbujas y brasa mientras uno da la calada
	int frame = 0;
	for(int i = 0; i < 2; i++)
	{
		int c = (time + SMOKERS[i].offset) % PUFF_PERIOD;
		if(c < INHALE_TIME)
			frame = 1 + (c / 160) % 2;
	}
	renderCell(bongQuads[0][frame], glm::vec2(146.f, 202.f), 4.f);

	// Humo: cada bocanada se calcula a partir del momento en que salio
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
			// sale hacia delante, frena y sube serpenteando
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

	// Fundido desde negro al entrar y a negro al salir
	int level = 0;
	if(time < FADE_IN_TIME)
		level = FADE_LEVELS - time * (FADE_LEVELS + 1) / FADE_IN_TIME;
	if(leaveTime >= 0)
		level = std::max(level, (leaveTime * (FADE_LEVELS + 1)) / FADE_OUT_TIME);
	fade.render(program, level);
}
