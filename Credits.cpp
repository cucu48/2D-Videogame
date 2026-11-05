#include <cmath>
#include <algorithm>
#include <string>
#include <GL/glew.h>
#include "Credits.h"


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


// Distancia (en px) de cada linea a la primera
static void lineOffsets(float offset[NUM_CREDITS])
{
	float acc = 0.f;
	for(int i = 0; i < NUM_CREDITS; i++)
	{
		offset[i] = acc;
		acc += (CREDITS[i].size > 0 ? CREDITS[i].size : BODY) + CREDITS_GAP;
	}
}


Credits::Credits()
{
	bText = false;
}

bool Credits::init()
{
	bText = text.init("images/bytebounce.medium.ttf");
	return bText;
}

// Lo que tienen que subir en total hasta que la ultima linea llega a lastY
float Credits::totalScroll(const CreditsLayout &layout) const
{
	float offset[NUM_CREDITS];
	lineOffsets(offset);
	return layout.bottom + offset[NUM_CREDITS - 1] - layout.lastY;
}

bool Credits::isFinished(int elapsed, const CreditsLayout &layout) const
{
	return elapsed * layout.speed >= totalScroll(layout);
}

void Credits::render(const glm::mat4 &projection, int elapsed, const CreditsLayout &layout)
{
	if(!bText || elapsed < 0)
		return;
	text.setProjection(projection);

	// se leen de arriba a abajo y suben; la ultima se para en lastY y se queda
	float offset[NUM_CREDITS];
	lineOffsets(offset);
	float scroll = std::min(elapsed * layout.speed, totalScroll(layout));

	for(int i = 0; i < NUM_CREDITS; i++)
	{
		const CreditLine &line = CREDITS[i];
		float base = std::floor(layout.bottom - scroll + offset[i]);
		if(line.size == 0 || base > layout.bottom + line.size || base < 0.f)
			continue;
		// se apagan al llegar arriba
		float alpha = (base - line.size - layout.fadeTo) / (layout.fadeFrom - layout.fadeTo);
		alpha = std::max(0.f, std::min(1.f, alpha));
		if(alpha <= 0.f)
			continue;
		std::string str(line.text);
		int width = 0;
		for(size_t c = 0; c < str.size(); c++)
			width += text.charAdvance(str[c], line.size);
		glm::vec2 pos(std::floor((layout.width - width) / 2.f), base);
		text.render(str, pos + glm::vec2(1.f, 1.f), line.size, glm::vec4(0.25f, 0.08f, 0.2f, alpha * 0.8f));
		text.render(str, pos, line.size, glm::vec4(line.r, line.g, line.b, alpha));
	}
}
