#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include "TextMenu.h"
#include "Audio.h"
#include "Sounds.h"


#define COLOR_NORMAL glm::vec4(0.85f, 0.85f, 0.85f, 1.f)
#define COLOR_SELECTED glm::vec4(1.f, 0.85f, 0.3f, 1.f)
#define COLOR_SHADOW glm::vec4(0.f, 0.f, 0.f, 0.85f)


TextMenu::TextMenu()
{
	bText = false;
	selected = 0;
	centerX = firstBaseline = spacing = 0.f;
	textSize = 12;
}

bool TextMenu::init()
{
	bText = text.init("images/bytebounce.medium.ttf");
	return bText;
}

void TextMenu::setItems(const std::vector<std::string> &newItems)
{
	items = newItems;
	if(selected >= int(items.size()))
		selected = 0;
}

void TextMenu::setItem(int index, const std::string &label)
{
	if(index >= 0 && index < int(items.size()))
		items[index] = label;
}

void TextMenu::setLayout(float cx, float baseline, int size, float space)
{
	centerX = cx;
	firstBaseline = baseline;
	textSize = size;
	spacing = space;
}

int TextMenu::keyPressed(int key)
{
	int n = int(items.size());
	if(n == 0)
		return -1;
	if(key == GLFW_KEY_UP || key == GLFW_KEY_W)
	{
		selected = (selected + n - 1) % n;
		Audio::instance().playSound(SFX_SELECT);
	}
	else if(key == GLFW_KEY_DOWN || key == GLFW_KEY_S)
	{
		selected = (selected + 1) % n;
		Audio::instance().playSound(SFX_SELECT);
	}
	else if(key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER || key == GLFW_KEY_SPACE)
		return selected;
	return -1;
}

int TextMenu::hover(const glm::vec2 &p)
{
	for(int i = 0; i < int(items.size()); i++)
	{
		float base = firstBaseline + i * spacing;
		float w = textWidth(items[i], textSize) + 24.f;   // un poco de margen a los lados
		if(p.y >= base - textSize - spacing * 0.25f && p.y < base + spacing * 0.25f &&
		   std::fabs(p.x - centerX) <= w / 2.f)
		{
			selected = i;
			return i;
		}
	}
	return -1;
}

float TextMenu::textWidth(const std::string &str, int size) const
{
	if(!bText)
		return 0.f;
	float w = 0.f;
	for(size_t c = 0; c < str.size(); c++)
		w += text.charAdvance(str[c], size);
	return w;
}

void TextMenu::renderLabel(const glm::mat4 &projection, const std::string &str, float cx, float baseline,
                           int size, const glm::vec4 &color)
{
	if(!bText)
		return;
	text.setProjection(projection);
	glm::vec2 pos(std::floor(cx - textWidth(str, size) / 2.f), std::floor(baseline));
	// contorno oscuro para que se lea sobre cualquier fondo
	const glm::vec2 around[4] = { glm::vec2(-1.f, 0.f), glm::vec2(1.f, 0.f), glm::vec2(0.f, -1.f), glm::vec2(0.f, 1.f) };
	for(int i = 0; i < 4; i++)
		text.render(str, pos + around[i], size, COLOR_SHADOW);
	text.render(str, pos + glm::vec2(1.f, 1.f), size, COLOR_SHADOW);
	text.render(str, pos, size, color);
}

void TextMenu::render(const glm::mat4 &projection)
{
	for(int i = 0; i < int(items.size()); i++)
	{
		float base = firstBaseline + i * spacing;
		if(i == selected)
		{
			// el elegido en amarillo y entre flechas
			renderLabel(projection, "> " + items[i] + " <", centerX, base, textSize, COLOR_SELECTED);
		}
		else
			renderLabel(projection, items[i], centerX, base, textSize, COLOR_NORMAL);
	}
}
