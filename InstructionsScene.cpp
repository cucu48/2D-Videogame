#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "InstructionsScene.h"
#include "Audio.h"
#include "Sounds.h"
#include "Game.h"


#define VIRTUAL_WIDTH 160
#define VIRTUAL_HEIGHT 144
#define TEXT_SCALE 2
#define DIM 0.3f
#define TEXT_TOP 24.f
#define BUTTON_Y 280.f


InstructionsScene::InstructionsScene()
{
	background = NULL;
}

InstructionsScene::~InstructionsScene()
{
	delete background;
}

void InstructionsScene::init()
{
	ShaderProgram &program = Game::instance().getTexProgram();
	backgroundTex.loadFromFile("images/MenuBG.png", TEXTURE_PIXEL_FORMAT_RGBA);
	backgroundTex.setMinFilter(GL_NEAREST);
	backgroundTex.setMagFilter(GL_NEAREST);
	background = Sprite::createSprite(glm::vec2(backgroundTex.width(), backgroundTex.height()), glm::vec2(1.f, 1.f),
	                                  &backgroundTex, &program);
	instructions.init();
	buttons.init();
	std::vector<std::string> items(1, "TORNAR");
	buttons.setItems(items);
	buttons.setLayout(VIRTUAL_WIDTH * TEXT_SCALE / 2.f, BUTTON_Y, 12, 18.f);
}

void InstructionsScene::onEnter()
{
	buttons.setSelected(0);
}

void InstructionsScene::update(int )
{
}

void InstructionsScene::render(int windowWidth, int windowHeight)
{
	glm::mat4 projection = setLetterboxViewport(windowWidth, windowHeight, VIRTUAL_WIDTH, VIRTUAL_HEIGHT);
	ShaderProgram &program = Game::instance().getTexProgram();
	program.use();
	program.setUniformMatrix4f("projection", projection);
	program.setUniform4f("color", DIM, DIM, DIM, 1.0f);
	background->render();

	glm::mat4 textProjection = glm::ortho(0.f, float(VIRTUAL_WIDTH * TEXT_SCALE), float(VIRTUAL_HEIGHT * TEXT_SCALE), 0.f);
	instructions.render(textProjection, VIRTUAL_WIDTH * TEXT_SCALE / 2.f, TEXT_TOP);
	buttons.render(textProjection);
}

void InstructionsScene::back()
{
	Audio::instance().playSound(SFX_SELECT);
	Game::instance().changeScene(SCENE_MENU);
}

void InstructionsScene::keyPressed(int key)
{
	if(key == GLFW_KEY_ESCAPE || buttons.keyPressed(key) >= 0)
		back();
}

void InstructionsScene::mouseMove(int x, int y)
{
	Game &game = Game::instance();
	glm::vec2 p;
	if(windowToVirtual(x, y, game.getWindowWidth(), game.getWindowHeight(), VIRTUAL_WIDTH * TEXT_SCALE, VIRTUAL_HEIGHT * TEXT_SCALE, p))
		buttons.hover(p);
}

void InstructionsScene::mousePress(int button)
{
	if(button != GLFW_MOUSE_BUTTON_LEFT)
		return;
	Game &game = Game::instance();
	glm::vec2 p;
	if(windowToVirtual(game.getMouseX(), game.getMouseY(), game.getWindowWidth(), game.getWindowHeight(),
	                   VIRTUAL_WIDTH * TEXT_SCALE, VIRTUAL_HEIGHT * TEXT_SCALE, p) && buttons.hover(p) >= 0)
		back();
}
