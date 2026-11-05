#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "VsScreen.h"
#include "Audio.h"
#include "Sounds.h"


#define SLIDE_IN_END 350     // ms: los paneles llegan al centro
#define SLAM_TIME 120        // ms que dura el golpe del VS (zoom y destello)
#define SHAKE_TIME 300       // ms que tiembla la pantalla tras el golpe
#define OUT_START 3400       // ms: empiezan a salir los paneles (total: OUT_START + OUT_TIME = 4 s)
#define OUT_TIME 600         // ms que tardan en salir
#define VS_W 320             // tamano de las imagenes de los paneles
#define VS_H 304


static float clamp01(float t) { return std::max(0.f, std::min(1.f, t)); }
static float easeOutBack(float t) { float c = 1.7f; t -= 1.f; return 1.f + (c + 1.f) * t * t * t + c * t * t; }
static float easeInCubic(float t) { return t * t * t; }


static TexturedQuad *makeQuad(float w, float h, ShaderProgram &program)
{
	glm::vec2 geom[2] = { glm::vec2(0.f, 0.f), glm::vec2(w, h) };
	glm::vec2 texCoords[2] = { glm::vec2(0.f, 0.f), glm::vec2(1.f, 1.f) };
	return TexturedQuad::createTexturedQuad(geom, texCoords, program);
}

static void loadTex(Texture &tex, const char *file)
{
	tex.loadFromFile(file, TEXTURE_PIXEL_FORMAT_RGBA);
	tex.setMinFilter(GL_NEAREST);
	tex.setMagFilter(GL_NEAREST);
}


VsScreen::VsScreen()
{
	leftQuad = rightQuad = vsQuad = blackQuad = NULL;
	time = 0;
	bActive = false;
	bSlamSound = false;
}

VsScreen::~VsScreen()
{
	TexturedQuad *quads[4] = { leftQuad, rightQuad, vsQuad, blackQuad };
	for(int i = 0; i < 4; i++)
		if(quads[i] != NULL)
		{
			quads[i]->free();
			delete quads[i];
		}
}

void VsScreen::init(ShaderProgram &program)
{
	loadTex(leftTex, "images/vs_left.png");
	loadTex(rightTex, "images/vs_right.png");
	loadTex(vsTex, "images/vs_text.png");
	loadTex(blackTex, "images/vs_black.png");
	leftQuad = makeQuad(VS_W, VS_H, program);
	rightQuad = makeQuad(VS_W, VS_H, program);
	vsQuad = makeQuad(float(vsTex.width()), float(vsTex.height()), program);
	blackQuad = makeQuad(VS_W, VS_H, program);
}

void VsScreen::start()
{
	time = 0;
	bActive = true;
	bSlamSound = false;
}

void VsScreen::skip()
{
	if(bActive && time < OUT_START)
		time = OUT_START;
}

void VsScreen::update(int deltaTime)
{
	if(!bActive)
		return;
	time += deltaTime;
	if(!bSlamSound && time >= SLIDE_IN_END)
	{
		Audio::instance().playSound(SFX_EXPLOSION);
		bSlamSound = true;
	}
	if(time >= OUT_START + OUT_TIME)
		bActive = false;
}

void VsScreen::render(ShaderProgram &program, int width, int height)
{
	if(!bActive)
		return;

	program.use();
	glm::mat4 projection = glm::ortho(0.f, float(width), float(height), 0.f);
	program.setUniformMatrix4f("projection", projection);
	program.setUniform2f("texCoordDispl", 0.f, 0.f);

	// Golpe del VS: destello blanco y temblor que se va apagando
	float sinceSlam = float(time - SLIDE_IN_END);
	bool flash = sinceSlam >= 0.f && sinceSlam < 70.f;
	glm::vec2 shake(0.f);
	if(sinceSlam >= 0.f && sinceSlam < SHAKE_TIME)
	{
		float amp = 4.f * (1.f - sinceSlam / SHAKE_TIME);
		shake = glm::vec2(float(rand() % 3 - 1), float(rand() % 3 - 1)) * amp;
	}
	float bright = flash ? 4.f : 1.f;   // el shader multiplica: >1 satura hacia blanco
	program.setUniform4f("color", bright, bright, bright, 1.f);

	// Paneles: entran desde los lados y salen en diagonal (izquierda arriba, derecha abajo)
	glm::vec2 leftPos, rightPos;
	if(time < SLIDE_IN_END)
	{
		float t = easeOutBack(clamp01(float(time) / SLIDE_IN_END));
		leftPos = glm::vec2(-width * (1.f - t), 0.f);
		rightPos = glm::vec2(width * (1.f - t), 0.f);
	}
	else if(time < OUT_START)
		leftPos = rightPos = glm::vec2(0.f);
	else
	{
		float t = easeInCubic(clamp01(float(time - OUT_START) / OUT_TIME));
		glm::vec2 dir(1.f, 0.45f);   // a lo largo de la diagonal de los paneles
		leftPos = -dir * (width * 1.3f * t);
		rightPos = dir * (width * 1.3f * t);
	}

	// Fondo negro hasta que empieza la salida: luego ya se ve el nivel por detras
	glm::mat4 base = glm::translate(glm::mat4(1.f), glm::vec3(glm::floor(shake), 0.f));
	if(time < OUT_START)
	{
		glm::mat4 m = glm::scale(glm::mat4(1.f), glm::vec3(float(width) / VS_W, float(height) / VS_H, 1.f));
		program.setUniformMatrix4f("modelview", m);
		blackQuad->render(blackTex);
	}
	glm::mat4 m = glm::translate(base, glm::vec3(glm::floor(leftPos), 0.f));
	program.setUniformMatrix4f("modelview", m);
	leftQuad->render(leftTex);
	m = glm::translate(base, glm::vec3(glm::floor(rightPos), 0.f));
	program.setUniformMatrix4f("modelview", m);
	rightQuad->render(rightTex);

	// VS: cae desde grande al chocar los paneles, late, y al salir crece y desaparece
	float scale = 0.f;
	if(sinceSlam >= 0.f && sinceSlam < SLAM_TIME)
		scale = 1.f + 3.f * (1.f - sinceSlam / SLAM_TIME);
	else if(sinceSlam >= SLAM_TIME && time < OUT_START)
		scale = 1.f + 0.06f * std::sin((sinceSlam - SLAM_TIME) * 0.012f);
	else if(time >= OUT_START && time < OUT_START + OUT_TIME / 4)
		scale = 1.f + 1.5f * (float(time - OUT_START) / (OUT_TIME / 4));
	if(scale > 0.f)
	{
		glm::vec2 size(vsTex.width() * scale, vsTex.height() * scale);
		// un poco por encima del centro, para no tapar la cabeza de Bomberman
		glm::vec2 pos = glm::floor(glm::vec2(width * 0.5f, height * 0.4f) - size / 2.f);
		glm::mat4 m = glm::translate(base, glm::vec3(pos, 0.f));
		m = glm::scale(m, glm::vec3(scale, scale, 1.f));
		program.setUniformMatrix4f("modelview", m);
		vsQuad->render(vsTex);
	}

	program.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
}
