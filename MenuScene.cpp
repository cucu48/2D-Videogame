#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "MenuScene.h"
#include "Game.h"


// The menu is drawn on a 160x144 virtual screen (the size of the art)
#define VIRTUAL_WIDTH 160
#define VIRTUAL_HEIGHT 144
#define BLINK_TIME 500


static Sprite *createImage(Texture &tex, const char *file, ShaderProgram &program)
{
	tex.loadFromFile(file, TEXTURE_PIXEL_FORMAT_RGBA);
	tex.setMinFilter(GL_NEAREST);
	tex.setMagFilter(GL_NEAREST);
	return Sprite::createSprite(glm::vec2(tex.width(), tex.height()), glm::vec2(1.f, 1.f), &tex, &program);
}


MenuScene::MenuScene()
{
	background = title = prompt = NULL;
	blinkTime = 0;
}

MenuScene::~MenuScene()
{
	delete background;
	delete title;
	delete prompt;
}

void MenuScene::init()
{
	ShaderProgram &program = Game::instance().getTexProgram();

	background = createImage(backgroundTex, "images/MenuBG.png", program);
	title = createImage(titleTex, "images/BombermanTitle.png", program);
	prompt = createImage(promptTex, "images/PressSpace.png", program);

	title->setPosition(glm::vec2((VIRTUAL_WIDTH - titleTex.width()) / 2, 8));
	prompt->setPosition(glm::vec2((VIRTUAL_WIDTH - promptTex.width()) / 2, 128));
}

void MenuScene::onEnter()
{
	blinkTime = 0;
}

void MenuScene::update(int deltaTime)
{
	blinkTime = (blinkTime + deltaTime) % (2 * BLINK_TIME);
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
	if(blinkTime < BLINK_TIME)
		prompt->render();
}

void MenuScene::keyPressed(int key)
{
	if(key == GLFW_KEY_SPACE)
		Game::instance().changeScene(SCENE_LEVEL_SELECT);
}
