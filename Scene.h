#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE


#include <glm/glm.hpp>
#include "ShaderProgram.h"


// Scene is the interface every screen of the game implements
// (main menu, level selector and the levels themselves).
// Game keeps a vector of them and forwards update/render/input to the current one.


class Scene
{

public:
	virtual ~Scene() {}

	virtual void init() = 0;                        // creates GL resources, called once at startup
	virtual void onEnter() {}                       // called every time the scene becomes the current one
	virtual void update(int deltaTime) = 0;
	virtual void render(int windowWidth, int windowHeight) = 0;
	virtual void keyPressed(int /*key*/) {}             // one call per key press (no auto repeat)
	virtual void mouseMove(int /*x*/, int /*y*/) {}     // x, y in window pixels, origin at the top-left
	virtual void mousePress(int /*button*/) {}          // GLFW_MOUSE_BUTTON_*
	virtual void mouseRelease(int /*button*/) {}

protected:
	// Sets a viewport of virtualWidth x virtualHeight (keeping its aspect ratio)
	// centered in the window and returns the matching pixel projection.
	static glm::mat4 setLetterboxViewport(int windowWidth, int windowHeight, int virtualWidth, int virtualHeight);

	// Converts a window position (pixels) to the virtual screen drawn by setLetterboxViewport.
	// Returns false if the position falls on the black bars.
	static bool windowToVirtual(int x, int y, int windowWidth, int windowHeight, int virtualWidth, int virtualHeight, glm::vec2 &result);

};


#endif // _SCENE_INCLUDE
