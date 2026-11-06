#include <cstdio>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include "DitherFade.h"


DitherFade::DitherFade()
{
	quad = NULL;
	width = height = 0;
}

DitherFade::~DitherFade()
{
	if(quad != NULL)
	{
		quad->free();
		delete quad;
	}
}

void DitherFade::init(ShaderProgram &program, int w, int h)
{
	width = w;
	height = h;
	for(int i = 0; i < FADE_LEVELS; i++)
	{
		char file[32];
		sprintf(file, "images/fade_%d.png", i + 1);
		tex[i].loadFromFile(file, TEXTURE_PIXEL_FORMAT_RGBA);
		tex[i].setMinFilter(GL_NEAREST);
		tex[i].setMagFilter(GL_NEAREST);
	}
	glm::vec2 geom[2] = { glm::vec2(0.f, 0.f), glm::vec2(float(w), float(h)) };
	glm::vec2 texCoords[2] = { glm::vec2(0.f, 0.f), glm::vec2(w / 8.f, h / 8.f) };
	quad = TexturedQuad::createTexturedQuad(geom, texCoords, program);
}

void DitherFade::render(ShaderProgram &program, int level)
{
	if(level <= 0)
		return;
	if(level > FADE_LEVELS)
		level = FADE_LEVELS;
	program.use();
	glm::mat4 projection = glm::ortho(0.f, float(width), float(height), 0.f);
	glm::mat4 modelview(1.f);
	program.setUniformMatrix4f("projection", projection);
	program.setUniformMatrix4f("modelview", modelview);
	program.setUniform2f("texCoordDispl", 0.f, 0.f);
	program.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
	quad->render(tex[level - 1]);
}
