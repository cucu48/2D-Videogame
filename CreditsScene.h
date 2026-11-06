#ifndef _CREDITS_SCENE_INCLUDE
#define _CREDITS_SCENE_INCLUDE


#include "Scene.h"
#include "Sprite.h"
#include "Texture.h"
#include "Credits.h"



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


#endif
