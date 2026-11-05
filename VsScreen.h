#ifndef _VS_SCREEN_INCLUDE
#define _VS_SCREEN_INCLUDE


#include "Texture.h"
#include "TexturedQuad.h"
#include "ShaderProgram.h"


// Pantalla "BOMBERMAN VS TRENT" al empezar el nivel del jefe, dibujada encima
// del nivel (que esta parado mientras dura). Las imagenes salen de
// scripts/make_vs_images.py. Secuencia:
//   1. los dos paneles entran de lado y chocan en la diagonal
//   2. golpe del "VS" (destello y temblor) y se queda un rato latiendo
//   3. los paneles salen disparados en diagonal y dejan ver el nivel


class VsScreen
{

public:
	VsScreen();
	~VsScreen();

	void init(ShaderProgram &program);
	void start();
	void skip();                 // salta directamente a la transicion de salida
	void update(int deltaTime);
	bool isActive() const { return bActive; }
	// Pinta en la pantalla virtual del nivel (width x height), con el viewport ya puesto
	void render(ShaderProgram &program, int width, int height);

private:
	Texture leftTex, rightTex, vsTex, blackTex;
	TexturedQuad *leftQuad, *rightQuad, *vsQuad, *blackQuad;
	int time;
	bool bActive, bSlamSound;

};


#endif // _VS_SCREEN_INCLUDE
