#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "LevelSelectScene.h"
#include "Audio.h"
#include "Sounds.h"


#define VIRTUAL_WIDTH 160
#define VIRTUAL_HEIGHT 144
#define BUTTON_SIZE 32
#define DIGIT_SIZE 16

static const glm::vec2 buttonPos[NUM_LEVELS] = {
	glm::vec2(128, 16), glm::vec2(0, 32), glm::vec2(128, 56), glm::vec2(0, 80), glm::vec2(128, 96)
};


static const glm::vec2 bongPos[NUM_BONGS] = {
	glm::vec2(110, 24), glm::vec2(34, 40), glm::vec2(110, 64), glm::vec2(34, 88)
};


static void loadTexture(Texture &tex, const char *file)
{
	tex.loadFromFile(file, TEXTURE_PIXEL_FORMAT_RGBA);
	tex.setMinFilter(GL_NEAREST);
	tex.setMagFilter(GL_NEAREST);
}


LevelSelectScene::LevelSelectScene()
{
	background = indicator = digit = bong = NULL;
	hovered = -1;
}

LevelSelectScene::~LevelSelectScene()
{
	delete background;
	delete indicator;
	delete digit;
	delete bong;
}

void LevelSelectScene::init()
{
	ShaderProgram &program = Game::instance().getTexProgram();

	loadTexture(backgroundTex, "images/LevelSelector.png");
	loadTexture(indicatorTex, "images/SelectedLevelIndicator.png");
	loadTexture(numbersTex, "images/NumbersOutlined.png");
	loadTexture(bongTex, "images/bong.png");
	bong = Sprite::createSprite(glm::vec2(16.f, 16.f), glm::vec2(0.5f, 1.f), &bongTex, &program);

	background = Sprite::createSprite(glm::vec2(VIRTUAL_WIDTH, VIRTUAL_HEIGHT), glm::vec2(1.f, 1.f), &backgroundTex, &program);
	indicator = Sprite::createSprite(glm::vec2(BUTTON_SIZE, BUTTON_SIZE), glm::vec2(1.f, 1.f), &indicatorTex, &program);

	digit = Sprite::createSprite(glm::vec2(DIGIT_SIZE, DIGIT_SIZE), glm::vec2(0.25f, 0.25f), &numbersTex, &program);
	digit->setNumberAnimations(NUM_LEVELS);
	for(int i = 0; i < NUM_LEVELS; i++)
	{
		int d = i + 1;
		digit->setAnimationSpeed(i, 1);
		digit->addKeyframe(i, glm::vec2((d % 4) * 0.25f, (d / 4) * 0.25f));
	}
}

void LevelSelectScene::onEnter()
{
	Game &game = Game::instance();
	updateHover(game.getMouseX(), game.getMouseY());
	Audio::instance().playMusic(MUSIC_TITLE, true);
}

void LevelSelectScene::update(int )
{
}

void LevelSelectScene::render(int windowWidth, int windowHeight)
{
	glm::mat4 projection = setLetterboxViewport(windowWidth, windowHeight, VIRTUAL_WIDTH, VIRTUAL_HEIGHT);

	ShaderProgram &program = Game::instance().getTexProgram();
	program.use();
	program.setUniformMatrix4f("projection", projection);
	program.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);

	background->render();

	for(int i = 0; i < NUM_BONGS; i++)
	{
		if(Game::instance().hasBong(i + 1))
			program.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
		else
			program.setUniform4f("color", 0.2f, 0.1f, 0.15f, 1.f);
		bong->setPosition(bongPos[i]);
		bong->render();
	}
	program.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);

	if(hovered >= 0)
	{
		indicator->setPosition(buttonPos[hovered]);
		indicator->render();
		digit->changeAnimation(hovered);
		digit->setPosition(buttonPos[hovered] + glm::vec2((BUTTON_SIZE - DIGIT_SIZE) / 2));
		digit->render();
	}
}

void LevelSelectScene::keyPressed(int key)
{
	if(key == GLFW_KEY_ESCAPE)
	{
		Game::instance().changeScene(SCENE_MENU);
		return;
	}
	if(key == GLFW_KEY_UP || key == GLFW_KEY_LEFT)
		hovered = (hovered < 0) ? NUM_LEVELS - 1 : (hovered + NUM_LEVELS - 1) % NUM_LEVELS;
	else if(key == GLFW_KEY_DOWN || key == GLFW_KEY_RIGHT)
		hovered = (hovered + 1) % NUM_LEVELS;
	else if((key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE) && hovered >= 0)
	{
		Audio::instance().playSound(SFX_SELECT);
		Game::instance().startLevel(hovered + 1);
	}
}

void LevelSelectScene::mouseMove(int x, int y)
{
	updateHover(x, y);
}

void LevelSelectScene::mousePress(int button)
{
	if(button == GLFW_MOUSE_BUTTON_LEFT && hovered >= 0)
	{
		Audio::instance().playSound(SFX_SELECT);
		Game::instance().startLevel(hovered + 1);
	}
}

void LevelSelectScene::updateHover(int x, int y)
{
	Game &game = Game::instance();
	glm::vec2 pos;
	hovered = -1;
	if(!windowToVirtual(x, y, game.getWindowWidth(), game.getWindowHeight(), VIRTUAL_WIDTH, VIRTUAL_HEIGHT, pos))
		return;
	for(int i = 0; i < NUM_LEVELS; i++)
		if(pos.x >= buttonPos[i].x && pos.x < buttonPos[i].x + BUTTON_SIZE &&
		   pos.y >= buttonPos[i].y && pos.y < buttonPos[i].y + BUTTON_SIZE)
			hovered = i;
}
