#ifndef _GAME_INCLUDE
#define _GAME_INCLUDE


#include <vector>
#include <GLFW/glfw3.h>
#include "Scene.h"
#include "ShaderProgram.h"


#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

// Position of each scene in Game's vector of scenes: menu, level selector
// and then one scene per level (level 1 at SCENE_FIRST_LEVEL, level 2 right after...)
#define SCENE_MENU 0
#define SCENE_LEVEL_SELECT 1
#define SCENE_FIRST_LEVEL 2
#define NUM_LEVELS 5


// Game is a singleton (a class with a single instance) that represents our whole application


class Game
{

private:
	Game() { currentScene = 0; mouseX = mouseY = -1; }
	~Game();
	
public:
	static Game &instance()
	{
		static Game G;
	
		return G;
	}
	
	void init();
	bool update(int deltaTime);
	void render();
	void resize(int width, int height);
	
	// Input callback methods
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMove(int x, int y);
	void mousePress(int button);
	void mouseRelease(int button);

	bool getKey(int key) const;
	int getMouseX() const { return mouseX; }
	int getMouseY() const { return mouseY; }
	int getWindowWidth() const { return windowWidth; }
	int getWindowHeight() const { return windowHeight; }

	// Scene management
	void changeScene(int index);
	void startLevel(int levelNumber); // levelNumber goes from 1 to NUM_LEVELS

	// Shader shared by all the scenes
	ShaderProgram &getTexProgram() { return texProgram; }

private:
	void initShaders();

private:
	bool bPlay; // Continue to play game?
	int windowWidth, windowHeight; // Tamaño real de la ventana en píxeles
	bool keys[GLFW_KEY_LAST+1]; // Store key states so that 
							    // we can have access at any time
	std::vector<Scene *> scenes;
	int currentScene;
	int mouseX, mouseY; // last cursor position, in window pixels
	ShaderProgram texProgram;

};


#endif // _GAME_INCLUDE