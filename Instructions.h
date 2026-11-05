#ifndef _INSTRUCTIONS_INCLUDE
#define _INSTRUCTIONS_INCLUDE


#include <string>
#include <glm/glm.hpp>
#include "Text.h"


// Texto de las instrucciones (en catalan): objetivo, controles, consejos y
// trucos. Lo usan la pantalla de instrucciones del menu y la de la pausa.
// Las lineas estan en Instructions.cpp.


class Instructions
{

public:
	Instructions();

	bool init();
	// Pinta el titulo y el texto centrados en centerX, empezando en la linea base top
	void render(const glm::mat4 &projection, float centerX, float top);

private:
	float textWidth(const std::string &str, int size) const;
	void label(const std::string &str, const glm::vec2 &pos, int size, const glm::vec4 &color);

private:
	Text text;
	bool bText;

};


#endif // _INSTRUCTIONS_INCLUDE
