#ifndef _DITHER_FADE_INCLUDE
#define _DITHER_FADE_INCLUDE


#include "Texture.h"
#include "TexturedQuad.h"
#include "ShaderProgram.h"


#define FADE_LEVELS 8


class DitherFade
{

public:
	DitherFade();
	~DitherFade();

	void init(ShaderProgram &program, int width, int height);
	void render(ShaderProgram &program, int level);

private:
	Texture tex[FADE_LEVELS];
	TexturedQuad *quad;
	int width, height;

};


#endif
