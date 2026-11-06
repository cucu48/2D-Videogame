#ifndef _GAME_INCLUDE
#define _GAME_INCLUDE


#include <vector>
#include <GLFW/glfw3.h>
#include "Scene.h"
#include "ShaderProgram.h"
#include "DitherFade.h"


#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define SCENE_MENU 0
#define SCENE_LEVEL_SELECT 1
#define SCENE_FIRST_LEVEL 2
#define NUM_LEVELS 5
#define SCENE_SECRET (SCENE_FIRST_LEVEL + NUM_LEVELS)
#define SCENE_INSTRUCTIONS (SCENE_SECRET + 1)
#define SCENE_CREDITS (SCENE_SECRET + 2)
#define START_LIVES 3
#define DEFAULT_MAX_BOMBS 1
#define DEFAULT_BLAST_RANGE 1
#define NUM_BONGS 4


struct PowerUpState
{
	int maxBombs;
	int blastRange;
	float speedMul;
	bool remote, wings, heart;

	PowerUpState() : maxBombs(DEFAULT_MAX_BOMBS), blastRange(DEFAULT_BLAST_RANGE), speedMul(1.f),
	                 remote(false), wings(false), heart(false) {}
};



class Game
{

private:
	Game() { currentScene = 0; suspendedScene = -1; pendingScene = NO_PENDING; fadeTime = 0; bFadingOut = false; mouseX = mouseY = -1; for(int i = 0; i < NUM_BONGS; i++) bongs[i] = false; }
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

	void changeScene(int index, bool immediate = false);
	bool isLevelScene(int index) const { return index >= SCENE_FIRST_LEVEL && index < SCENE_FIRST_LEVEL + NUM_LEVELS; }
	void quit() { bPlay = false; }

	void suspendToMenu();
	bool hasSuspendedGame() const { return suspendedScene >= 0; }
	void resumeSuspendedGame();
	void startLevel(int levelNumber);

	int getLives() const { return lives; }
	void loseLife() { if(lives > 0) lives--; }

	PowerUpState &powerUps() { return powerUpState; }

	bool isGodMode() const { return bGodMode; }
	void toggleGodMode() { bGodMode = !bGodMode; }

	bool hasBong(int levelNumber) const { return levelNumber >= 1 && levelNumber <= NUM_BONGS && bongs[levelNumber - 1]; }
	void collectBong(int levelNumber) { if(levelNumber >= 1 && levelNumber <= NUM_BONGS) bongs[levelNumber - 1] = true; }
	void resetBongs() { for(int i = 0; i < NUM_BONGS; i++) bongs[i] = false; }
	int bongCount() const { int n = 0; for(int i = 0; i < NUM_BONGS; i++) n += bongs[i] ? 1 : 0; return n; }

	ShaderProgram &getTexProgram() { return texProgram; }

private:
	void initShaders();
	void doChangeScene(int index);
	void doResume();

private:
	bool bPlay;
	bool bongs[NUM_BONGS];
	int windowWidth, windowHeight;
	bool keys[GLFW_KEY_LAST+1];
	std::vector<Scene *> scenes;
	int currentScene;
	int suspendedScene;
	int suspendedLives;
	PowerUpState suspendedPowerUps;
	int lives;
	PowerUpState powerUpState;
	bool bGodMode;
	int mouseX, mouseY;
	enum { NO_PENDING = -1, PENDING_RESUME = -2 };
	int pendingScene;
	int fadeTime;
	bool bFadingOut;
	DitherFade transition;
	ShaderProgram texProgram;

};


#endif
