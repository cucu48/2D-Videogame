#ifndef _TEXT_MENU_INCLUDE
#define _TEXT_MENU_INCLUDE


#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "Text.h"



class TextMenu
{

public:
	TextMenu();

	bool init();
	void setItems(const std::vector<std::string> &items);
	void setItem(int index, const std::string &label);
	void setLayout(float centerX, float firstBaseline, int size, float spacing);
	int getSelected() const { return selected; }
	void setSelected(int index) { selected = index; }
	int size() const { return int(items.size()); }

	int keyPressed(int key);
	int hover(const glm::vec2 &point);
	float itemCenterX() const { return centerX; }

	void render(const glm::mat4 &projection);
	void renderLabel(const glm::mat4 &projection, const std::string &str, float centerX, float baseline,
	                 int size, const glm::vec4 &color);

private:
	float textWidth(const std::string &str, int size) const;

private:
	Text text;
	bool bText;
	std::vector<std::string> items;
	int selected;
	float centerX, firstBaseline, spacing;
	int textSize;

};


#endif
