#ifndef _MENU_SCENE_INCLUDE
#define _MENU_SCENE_INCLUDE


#include "Scene.h"
#include "Sprite.h"
#include "Texture.h"


// Main menu: background, title and a blinking "press space" message.


class MenuScene : public Scene
{

public:
	MenuScene();
	~MenuScene();

	void init();
	void onEnter();
	void update(int deltaTime);
	void render(int windowWidth, int windowHeight);
	void keyPressed(int key);

private:
	Texture backgroundTex, titleTex, promptTex;
	Sprite *background, *title, *prompt;
	int blinkTime;

};


#endif // _MENU_SCENE_INCLUDE
