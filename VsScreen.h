#ifndef _VS_SCREEN_INCLUDE
#define _VS_SCREEN_INCLUDE


#include "Texture.h"
#include "TexturedQuad.h"
#include "ShaderProgram.h"



class VsScreen
{

public:
	VsScreen();
	~VsScreen();

	void init(ShaderProgram &program);
	void start();
	void skip();
	void update(int deltaTime);
	bool isActive() const { return bActive; }
	void render(ShaderProgram &program, int width, int height);

private:
	Texture leftTex, rightTex, vsTex, blackTex;
	TexturedQuad *leftQuad, *rightQuad, *vsQuad, *blackQuad;
	int time;
	bool bActive, bSlamSound;

};


#endif
