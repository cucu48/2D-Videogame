#ifndef _MENU_SCENE_INCLUDE
#define _MENU_SCENE_INCLUDE


#include "Scene.h"
#include "Sprite.h"
#include "Texture.h"
#include "TextMenu.h"



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
	void mouseMove(int x, int y);
	void mousePress(int button);

private:
	void choose(int item);

	Texture backgroundTex, titleTex;
	Sprite *background, *title;
	TextMenu buttons;
	bool bContinue;

};


#endif
