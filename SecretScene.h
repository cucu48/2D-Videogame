#ifndef _SECRET_SCENE_INCLUDE
#define _SECRET_SCENE_INCLUDE


#include "Scene.h"
#include "Texture.h"
#include "TexturedQuad.h"
#include "Sprite.h"
#include "DitherFade.h"
#include "Credits.h"



class SecretScene : public Scene
{

public:
	SecretScene();
	~SecretScene();

	void init();
	void onEnter();
	void update(int deltaTime);
	void render(int windowWidth, int windowHeight);
	void keyPressed(int key);

private:
	void renderCell(TexturedQuad *quad, const glm::vec2 &pos, float scale);
	void renderCredits(const glm::mat4 &projection);

private:
	Texture bgTex, fgTex, fxTex, promptTex;
	TexturedQuad *bgQuad, *fgQuad, *promptQuad;
	Credits credits;
	TexturedQuad *bongQuads[2][3];
	TexturedQuad *smokeQuads[4];
	DitherFade fade;
	int time;
	int leaveTime;
	bool bInScene;

};


#endif
