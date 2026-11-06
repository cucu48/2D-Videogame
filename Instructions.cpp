#include <cmath>
#include <GL/glew.h>
#include "Instructions.h"


enum LineKind { HEADER, TEXT, ROW, GAP };

struct HelpLine
{
	LineKind kind;
	const char *a;
	const char *b;
};

static const HelpLine HELP[] = {
	{ HEADER, "OBJECTIU", "" },
	{ TEXT, "ELIMINA TOTS ELS ENEMICS I SURT", "" },
	{ TEXT, "PER LA PORTA ABANS QUE ACABI EL TEMPS", "" },
	{ TEXT, "ELS ARBUSTOS AMAGUEN POWER-UPS", "" },
	{ TEXT, "TROBA ELS 4 BONGS PER UN FINAL SECRET", "" },
	{ GAP, "", "" },
	{ HEADER, "CONTROLS", "" },
	{ ROW, "FLETXES", "MOURE'S" },
	{ ROW, "AMUNT", "SALTAR" },
	{ ROW, "F", "POSAR BOMBA" },
	{ ROW, "D", "DETONAR (COMANDAMENT)" },
	{ ROW, "B", "BALLAR" },
	{ ROW, "ESC", "PAUSA" },
	{ GAP, "", "" },
	{ HEADER, "TRUCS", "" },
	{ ROW, "G", "MODE DEU" },
	{ ROW, "K", "MATAR ELS ENEMICS" },
	{ ROW, "E", "TOTS ELS BONGS" },
	{ ROW, "1-5", "ANAR A UN NIVELL" },
};
#define NUM_HELP int(sizeof(HELP) / sizeof(HELP[0]))

#define TITLE_SIZE 16
#define HEADER_SIZE 12
#define BODY_SIZE 10
#define TITLE_SPACING 22.f
#define HEADER_SPACING 14.f
#define BODY_SPACING 12.f
#define GAP_SPACING 4.f
#define ROW_GAP 8.f

static const glm::vec4 GOLD(1.f, 0.85f, 0.35f, 1.f);
static const glm::vec4 CREAM(1.f, 0.95f, 0.85f, 1.f);
static const glm::vec4 KEY_COLOR(0.55f, 0.85f, 1.f, 1.f);


Instructions::Instructions()
{
	bText = false;
}

bool Instructions::init()
{
	bText = text.init("images/bytebounce.medium.ttf");
	return bText;
}

float Instructions::textWidth(const std::string &str, int size) const
{
	int width = 0;
	for(size_t i = 0; i < str.size(); i++)
		width += text.charAdvance(str[i], size);
	return float(width);
}

void Instructions::label(const std::string &str, const glm::vec2 &pos, int size, const glm::vec4 &color)
{
	glm::vec2 p(std::floor(pos.x), std::floor(pos.y));
	glm::vec4 shadow(0.1f, 0.05f, 0.1f, color.a);
	text.render(str, p + glm::vec2(1.f, 0.f), size, shadow);
	text.render(str, p + glm::vec2(-1.f, 0.f), size, shadow);
	text.render(str, p + glm::vec2(0.f, 1.f), size, shadow);
	text.render(str, p + glm::vec2(0.f, -1.f), size, shadow);
	text.render(str, p, size, color);
}

void Instructions::render(const glm::mat4 &projection, float centerX, float top)
{
	if(!bText)
		return;
	text.setProjection(projection);

	std::string title("INSTRUCCIONS");
	label(title, glm::vec2(centerX - textWidth(title, TITLE_SIZE) / 2.f, top), TITLE_SIZE, glm::vec4(1.f));

	float y = top + TITLE_SPACING;
	for(int i = 0; i < NUM_HELP; i++)
	{
		const HelpLine &line = HELP[i];
		std::string a(line.a), b(line.b);
		if(line.kind == GAP)
		{
			y += GAP_SPACING;
			continue;
		}
		if(line.kind == HEADER)
		{
			label(a, glm::vec2(centerX - textWidth(a, HEADER_SIZE) / 2.f, y), HEADER_SIZE, GOLD);
			y += HEADER_SPACING;
			continue;
		}
		if(line.kind == TEXT)
			label(a, glm::vec2(centerX - textWidth(a, BODY_SIZE) / 2.f, y), BODY_SIZE, CREAM);
		else
		{
			label(a, glm::vec2(centerX - ROW_GAP / 2.f - textWidth(a, BODY_SIZE), y), BODY_SIZE, KEY_COLOR);
			label(b, glm::vec2(centerX + ROW_GAP / 2.f, y), BODY_SIZE, CREAM);
		}
		y += BODY_SPACING;
	}
}
