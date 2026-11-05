#ifndef _DITHER_FADE_INCLUDE
#define _DITHER_FADE_INCLUDE


#include "Texture.h"
#include "TexturedQuad.h"
#include "ShaderProgram.h"


// Fundido a negro con trama (el shader no mezcla transparencias): 8 niveles,
// images/fade_1..8.png (patrones Bayer de 8x8 que se repiten por la pantalla).
// Nivel 0 = no se ve nada, nivel 8 = negro del todo.
#define FADE_LEVELS 8


class DitherFade
{

public:
	DitherFade();
	~DitherFade();

	void init(ShaderProgram &program, int width, int height);
	// Pinta la trama del nivel dado (0..FADE_LEVELS) encima de todo, con el viewport ya puesto
	void render(ShaderProgram &program, int level);

private:
	Texture tex[FADE_LEVELS];
	TexturedQuad *quad;
	int width, height;

};


#endif // _DITHER_FADE_INCLUDE
