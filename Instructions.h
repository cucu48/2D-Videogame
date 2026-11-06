#ifndef _INSTRUCTIONS_INCLUDE
#define _INSTRUCTIONS_INCLUDE


#include <string>
#include <glm/glm.hpp>
#include "Text.h"



class Instructions
{

public:
	Instructions();

	bool init();
	void render(const glm::mat4 &projection, float centerX, float top);

private:
	float textWidth(const std::string &str, int size) const;
	void label(const std::string &str, const glm::vec2 &pos, int size, const glm::vec4 &color);

private:
	Text text;
	bool bText;

};


#endif
