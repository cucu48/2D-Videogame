#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "LevelSelectScene.h"


// The selector is drawn on a 160x144 virtual screen (the size of the art)
#define VIRTUAL_WIDTH 160
#define VIRTUAL_HEIGHT 144
#define BUTTON_SIZE 32
#define DIGIT_SIZE 16

// Top-left corner of the buttons painted on LevelSelector.png (level 1 to 5)
static const glm::vec2 buttonPos[NUM_LEVELS] = {
	glm::vec2(128, 16), glm::vec2(0, 32), glm::vec2(128, 56), glm::vec2(0, 80), glm::vec2(128, 96)
};


static void loadTexture(Texture &tex, const char *file)
{
	tex.loadFromFile(file, TEXTURE_PIXEL_FORMAT_RGBA);
	tex.setMinFilter(GL_NEAREST);
	tex.setMagFilter(GL_NEAREST);
}


LevelSelectScene::LevelSelectScene()
{
	background = indicator = digit = NULL;
	hovered = -1;
}

LevelSelectScene::~LevelSelectScene()
{
	delete background;
	delete indicator;
	delete digit;
}

void LevelSelectScene::init()
{
	ShaderProgram &program = Game::instance().getTexProgram();

	loadTexture(backgroundTex, "images/LevelSelector.png");
	loadTexture(indicatorTex, "images/SelectedLevelIndicator.png");
	loadTexture(numbersTex, "images/NumbersOutlined.png");

	background = Sprite::createSprite(glm::vec2(VIRTUAL_WIDTH, VIRTUAL_HEIGHT), glm::vec2(1.f, 1.f), &backgroundTex, &program);
	indicator = Sprite::createSprite(glm::vec2(BUTTON_SIZE, BUTTON_SIZE), glm::vec2(1.f, 1.f), &indicatorTex, &program);

	// NumbersOutlined.png holds the digits 0-9 in 16x16 cells, 4 per row. One animation per level number.
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
	// the cursor may already be over a button when the scene is entered
	Game &game = Game::instance();
	updateHover(game.getMouseX(), game.getMouseY());
}

void LevelSelectScene::update(int /*deltaTime*/)
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

	// Highlight frame over the hovered button, with its number on top
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
	if(key == GLFW_KEY_UP || key == GLFW_KEY_LEFT)
		hovered = (hovered < 0) ? NUM_LEVELS - 1 : (hovered + NUM_LEVELS - 1) % NUM_LEVELS;
	else if(key == GLFW_KEY_DOWN || key == GLFW_KEY_RIGHT)
		hovered = (hovered + 1) % NUM_LEVELS;
	else if((key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE) && hovered >= 0)
		Game::instance().startLevel(hovered + 1);
}

void LevelSelectScene::mouseMove(int x, int y)
{
	updateHover(x, y);
}

void LevelSelectScene::mousePress(int button)
{
	if(button == GLFW_MOUSE_BUTTON_LEFT && hovered >= 0)
		Game::instance().startLevel(hovered + 1);
}

// Highlights the button under the window position (x, y), or none
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
