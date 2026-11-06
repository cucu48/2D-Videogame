#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE


#include <glm/glm.hpp>
#include "ShaderProgram.h"



class Scene
{

public:
	virtual ~Scene() {}

	virtual void init() = 0;
	virtual void onEnter() {}
	virtual void onResume() {}
	virtual void update(int deltaTime) = 0;
	virtual void render(int windowWidth, int windowHeight) = 0;
	virtual void keyPressed(int ) {}
	virtual void mouseMove(int , int ) {}
	virtual void mousePress(int ) {}
	virtual void mouseRelease(int ) {}

protected:
	static glm::mat4 setLetterboxViewport(int windowWidth, int windowHeight, int virtualWidth, int virtualHeight);

	static bool windowToVirtual(int x, int y, int windowWidth, int windowHeight, int virtualWidth, int virtualHeight, glm::vec2 &result);

};


#endif
