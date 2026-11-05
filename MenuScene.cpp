#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "MenuScene.h"
#include "Audio.h"
#include "Sounds.h"
#include "Game.h"


// The menu is drawn on a 160x144 virtual screen (the size of the art). The
// buttons use twice that resolution so the text looks like in the levels.
#define VIRTUAL_WIDTH 160
#define VIRTUAL_HEIGHT 144
#define TEXT_SCALE 2
#define BUTTON_SIZE 14
#define BUTTON_SPACING 18.f
#define BUTTONS_BOTTOM 278.f    // baseline of the last button (in text pixels)


static Sprite *createImage(Texture &tex, const char *file, ShaderProgram &program)
{
	tex.loadFromFile(file, TEXTURE_PIXEL_FORMAT_RGBA);
	tex.setMinFilter(GL_NEAREST);
	tex.setMagFilter(GL_NEAREST);
	return Sprite::createSprite(glm::vec2(tex.width(), tex.height()), glm::vec2(1.f, 1.f), &tex, &program);
}


MenuScene::MenuScene()
{
	background = title = NULL;
	bContinue = false;
}

MenuScene::~MenuScene()
{
	delete background;
	delete title;
}

void MenuScene::init()
{
	ShaderProgram &program = Game::instance().getTexProgram();

	background = createImage(backgroundTex, "images/MenuBG.png", program);
	title = createImage(titleTex, "images/BombermanTitle.png", program);
	title->setPosition(glm::vec2((VIRTUAL_WIDTH - titleTex.width()) / 2, 8));
	buttons.init();
}

void MenuScene::onEnter()
{
	Audio::instance().setMusicDucked(false);
	Audio::instance().setMusicHurry(false);
	Audio::instance().playMusic(MUSIC_TITLE, true);
	// With a game left from the pause menu, it can be continued
	bContinue = Game::instance().hasSuspendedGame();
	std::vector<std::string> items;
	if(bContinue)
		items.push_back("CONTINUAR");
	items.push_back(bContinue ? "NOVA PARTIDA" : "JUGAR");
	items.push_back("INSTRUCCIONS");
	items.push_back("CREDITS");
	items.push_back("SORTIR");
	buttons.setItems(items);
	buttons.setSelected(0);
	float first = BUTTONS_BOTTOM - (items.size() - 1) * BUTTON_SPACING;
	buttons.setLayout(VIRTUAL_WIDTH * TEXT_SCALE / 2.f, first, BUTTON_SIZE, BUTTON_SPACING);
}

void MenuScene::update(int /*deltaTime*/)
{
}

void MenuScene::render(int windowWidth, int windowHeight)
{
	glm::mat4 projection = setLetterboxViewport(windowWidth, windowHeight, VIRTUAL_WIDTH, VIRTUAL_HEIGHT);

	ShaderProgram &program = Game::instance().getTexProgram();
	program.use();
	program.setUniformMatrix4f("projection", projection);
	program.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);

	background->render();
	title->render();

	glm::mat4 textProjection = glm::ortho(0.f, float(VIRTUAL_WIDTH * TEXT_SCALE), float(VIRTUAL_HEIGHT * TEXT_SCALE), 0.f);
	buttons.render(textProjection);
}

void MenuScene::choose(int item)
{
	if(item < 0)
		return;
	Audio::instance().playSound(SFX_SELECT);
	if(!bContinue)
		item++;   // without "Continuar" the buttons start at "Jugar"
	if(item == 0)
		Game::instance().resumeSuspendedGame();
	else if(item == 1)
	{
		// "Nova partida" (habia una partida en pausa): se empieza sin bongs
		if(bContinue)
			Game::instance().resetBongs();
		Game::instance().changeScene(SCENE_LEVEL_SELECT);
	}
	else if(item == 2)
		Game::instance().changeScene(SCENE_INSTRUCTIONS);
	else if(item == 3)
		Game::instance().changeScene(SCENE_CREDITS);
	else
		Game::instance().quit();
}

void MenuScene::keyPressed(int key)
{
	choose(buttons.keyPressed(key));
}

void MenuScene::mouseMove(int x, int y)
{
	Game &game = Game::instance();
	glm::vec2 p;
	if(windowToVirtual(x, y, game.getWindowWidth(), game.getWindowHeight(), VIRTUAL_WIDTH * TEXT_SCALE, VIRTUAL_HEIGHT * TEXT_SCALE, p))
		buttons.hover(p);
}

void MenuScene::mousePress(int button)
{
	if(button != GLFW_MOUSE_BUTTON_LEFT)
		return;
	Game &game = Game::instance();
	glm::vec2 p;
	if(windowToVirtual(game.getMouseX(), game.getMouseY(), game.getWindowWidth(), game.getWindowHeight(),
	                   VIRTUAL_WIDTH * TEXT_SCALE, VIRTUAL_HEIGHT * TEXT_SCALE, p))
		choose(buttons.hover(p));
}
