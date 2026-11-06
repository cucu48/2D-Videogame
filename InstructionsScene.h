#ifndef _INSTRUCTIONS_SCENE_INCLUDE
#define _INSTRUCTIONS_SCENE_INCLUDE


#include "Scene.h"
#include "Sprite.h"
#include "Texture.h"
#include "TextMenu.h"
#include "Instructions.h"



class InstructionsScene : public Scene
{

public:
	InstructionsScene();
	~InstructionsScene();

	void init();
	void onEnter();
	void update(int deltaTime);
	void render(int windowWidth, int windowHeight);
	void keyPressed(int key);
	void mouseMove(int x, int y);
	void mousePress(int button);

private:
	void back();

	Texture backgroundTex;
	Sprite *background;
	Instructions instructions;
	TextMenu buttons;

};


#endif
