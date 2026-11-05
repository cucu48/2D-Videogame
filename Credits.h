#ifndef _CREDITS_INCLUDE
#define _CREDITS_INCLUDE


#include <glm/glm.hpp>
#include "Text.h"


// Texto de los creditos (en catalan) que sube por la pantalla. Lo usan el
// final secreto (sale de detras de las colinas) y la pantalla de creditos del
// menu (sobre el fondo del menu). La lista de lineas esta en Credits.cpp.


struct CreditsLayout
{
	float width;       // ancho de la pantalla virtual (para centrar)
	float bottom;      // y (linea base) donde nace cada linea
	float fadeFrom;    // por encima de aqui (parte de arriba del texto) se van apagando...
	float fadeTo;      // ...y aqui ya no se ven
	float lastY;       // la ultima linea se para aqui y se queda
	float speed;       // px por ms
};


class Credits
{

public:
	Credits();

	bool init();
	// elapsed: ms desde que empezaron (negativo = aun no)
	void render(const glm::mat4 &projection, int elapsed, const CreditsLayout &layout);
	// true cuando la ultima linea ya ha llegado a su sitio
	bool isFinished(int elapsed, const CreditsLayout &layout) const;

private:
	float totalScroll(const CreditsLayout &layout) const;

private:
	Text text;
	bool bText;

};


#endif // _CREDITS_INCLUDE
