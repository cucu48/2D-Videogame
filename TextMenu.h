#ifndef _TEXT_MENU_INCLUDE
#define _TEXT_MENU_INCLUDE


#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "Text.h"


// Lista vertical de botones de texto centrados (pausa, opciones, menu
// principal). Se elige con arriba/abajo y espacio/enter, o con el raton.
// Las posiciones van en pixeles de la pantalla virtual que se le pase a render.


class TextMenu
{

public:
	TextMenu();

	bool init();                                   // carga la fuente
	void setItems(const std::vector<std::string> &items);
	void setItem(int index, const std::string &label);
	void setLayout(float centerX, float firstBaseline, int size, float spacing);
	int getSelected() const { return selected; }
	void setSelected(int index) { selected = index; }
	int size() const { return int(items.size()); }

	// Arriba/abajo mueven la seleccion (con sonido). Devuelve el boton
	// activado con espacio/enter, o -1.
	int keyPressed(int key);
	// Boton bajo el punto (pixeles virtuales), o -1. Si hay uno, lo selecciona.
	int hover(const glm::vec2 &point);
	// Centro x de los botones (para saber si el raton esta a la izquierda o a la derecha)
	float itemCenterX() const { return centerX; }

	void render(const glm::mat4 &projection);
	// Texto suelto centrado con sombra (titulos)
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


#endif // _TEXT_MENU_INCLUDE
