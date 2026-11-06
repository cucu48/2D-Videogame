#ifndef _LEVEL_SELECT_SCENE_INCLUDE
#define _LEVEL_SELECT_SCENE_INCLUDE


#include "Scene.h"
#include "Game.h"
#include "Sprite.h"
#include "Texture.h"



class LevelSelectScene : public Scene
{

public:
	LevelSelectScene();
	~LevelSelectScene();

	void init();
	void onEnter();
	void update(int deltaTime);
	void render(int windowWidth, int windowHeight);
	void keyPressed(int key);
	void mouseMove(int x, int y);
	void mousePress(int button);

private:
	Texture backgroundTex, indicatorTex, numbersTex, bongTex;
	Sprite *background, *indicator, *digit, *bong;
	void updateHover(int x, int y);

	int hovered;

};


#endif
