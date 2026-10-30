#ifndef _LEVEL_SELECT_SCENE_INCLUDE
#define _LEVEL_SELECT_SCENE_INCLUDE


#include "Scene.h"
#include "Game.h"
#include "Sprite.h"
#include "Texture.h"


// Level selector: hover a level with the mouse to highlight it and click to play it.
// Number keys (handled by Game) and the arrows + enter/space also work.


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
	Texture backgroundTex, indicatorTex, numbersTex;
	Sprite *background, *indicator, *digit;
	void updateHover(int x, int y);

	int hovered; // 0 .. NUM_LEVELS-1, -1 if no level is highlighted

};


#endif // _LEVEL_SELECT_SCENE_INCLUDE
