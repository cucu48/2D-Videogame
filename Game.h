#ifndef _GAME_INCLUDE
#define _GAME_INCLUDE


#include <vector>
#include <GLFW/glfw3.h>
#include "Scene.h"
#include "ShaderProgram.h"
#include "DitherFade.h"


#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

// Position of each scene in Game's vector of scenes: menu, level selector
// and then one scene per level (level 1 at SCENE_FIRST_LEVEL, level 2 right after...)
#define SCENE_MENU 0
#define SCENE_LEVEL_SELECT 1
#define SCENE_FIRST_LEVEL 2
#define NUM_LEVELS 5
#define SCENE_SECRET (SCENE_FIRST_LEVEL + NUM_LEVELS)   // final secreto, despues de los niveles
#define SCENE_INSTRUCTIONS (SCENE_SECRET + 1)
#define SCENE_CREDITS (SCENE_SECRET + 2)
#define START_LIVES 3
#define DEFAULT_MAX_BOMBS 1
#define DEFAULT_BLAST_RANGE 1
#define NUM_BONGS 4             // coleccionables: uno escondido en cada uno de los stages 1-4


// Power-ups that stay with the player from one level to the next (like lives).
// Timed ones (armor, timer freeze) are not kept.
struct PowerUpState
{
	int maxBombs;
	int blastRange;
	float speedMul;
	bool remote, wings, heart;

	PowerUpState() : maxBombs(DEFAULT_MAX_BOMBS), blastRange(DEFAULT_BLAST_RANGE), speedMul(1.f),
	                 remote(false), wings(false), heart(false) {}
};


// Game is a singleton (a class with a single instance) that represents our whole application


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

	// Scene management. Normally the screen fades out (dithered) to black, the
	// scene changes and it fades in again. immediate skips the fade.
	void changeScene(int index, bool immediate = false);
	bool isLevelScene(int index) const { return index >= SCENE_FIRST_LEVEL && index < SCENE_FIRST_LEVEL + NUM_LEVELS; }
	void quit() { bPlay = false; }

	// "Menu principal" desde la pausa: el nivel se queda tal cual (con sus
	// power-ups, vidas, enemigos...) y desde el menu se puede continuar.
	// Empezar otro nivel la descarta.
	void suspendToMenu();
	bool hasSuspendedGame() const { return suspendedScene >= 0; }
	void resumeSuspendedGame();
	void startLevel(int levelNumber); // levelNumber goes from 1 to NUM_LEVELS

	// Lives last the whole game (they carry over between levels). Leaving the
	// gameplay (menu or level selector) starts a new game with START_LIVES.
	int getLives() const { return lives; }
	void loseLife() { if(lives > 0) lives--; }

	// Power-ups carry over between levels and reset together with the lives
	PowerUpState &powerUps() { return powerUpState; }

	// Modo dios (tecla G): el jugador no puede morir. Se mantiene entre niveles.
	bool isGodMode() const { return bGodMode; }
	void toggleGodMode() { bGodMode = !bGodMode; }

	// Coleccionables (bongs): uno por stage 1-4. Se guardan mientras el juego
	// esta abierto, aunque se pierda la partida.
	bool hasBong(int levelNumber) const { return levelNumber >= 1 && levelNumber <= NUM_BONGS && bongs[levelNumber - 1]; }
	void collectBong(int levelNumber) { if(levelNumber >= 1 && levelNumber <= NUM_BONGS) bongs[levelNumber - 1] = true; }
	void resetBongs() { for(int i = 0; i < NUM_BONGS; i++) bongs[i] = false; }
	int bongCount() const { int n = 0; for(int i = 0; i < NUM_BONGS; i++) n += bongs[i] ? 1 : 0; return n; }

	// Shader shared by all the scenes
	ShaderProgram &getTexProgram() { return texProgram; }

private:
	void initShaders();
	void doChangeScene(int index);
	void doResume();

private:
	bool bPlay; // Continue to play game?
	bool bongs[NUM_BONGS];
	int windowWidth, windowHeight; // Tamaño real de la ventana en píxeles
	bool keys[GLFW_KEY_LAST+1]; // Store key states so that 
							    // we can have access at any time
	std::vector<Scene *> scenes;
	int currentScene;
	int suspendedScene;             // nivel dejado desde la pausa, o -1
	int suspendedLives;
	PowerUpState suspendedPowerUps;
	int lives;
	PowerUpState powerUpState;
	bool bGodMode;
	int mouseX, mouseY; // last cursor position, in window pixels
	// Transicion entre escenas: fundido a negro (la escena se queda quieta y
	// sin input), cambio, y fundido de vuelta
	enum { NO_PENDING = -1, PENDING_RESUME = -2 };
	int pendingScene;
	int fadeTime;                   // ms en la fase actual del fundido
	bool bFadingOut;
	DitherFade transition;
	ShaderProgram texProgram;

};


#endif // _GAME_INCLUDE
