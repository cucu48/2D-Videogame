#ifndef _CREDITS_SCENE_INCLUDE
#define _CREDITS_SCENE_INCLUDE


#include "Scene.h"
#include "Sprite.h"
#include "Texture.h"
#include "Credits.h"


// Creditos sobre el fondo del menu principal (oscurecido), con la musica de
// los creditos. Se llega desde el menu y al acabar el juego sin el final
// secreto. Espacio, enter o Esc vuelven al menu.


class CreditsScene : public Scene
{

public:
	CreditsScene();
	~CreditsScene();

	void init();
	void onEnter();
	void update(int deltaTime);
	void render(int windowWidth, int windowHeight);
	void keyPressed(int key);
	void mousePress(int button);

private:
	void leave();

	Texture backgroundTex, promptTex;
	Sprite *background, *prompt;
	Credits credits;
	int time;
	bool bLeaving;

};


#endif // _CREDITS_SCENE_INCLUDE
