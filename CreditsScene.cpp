#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "CreditsScene.h"
#include "Audio.h"
#include "Sounds.h"
#include "Game.h"


#define VIRTUAL_WIDTH 160
#define VIRTUAL_HEIGHT 144
#define TEXT_SCALE 2
#define DIM 0.4f
#define START_DELAY 600
#define SKIP_TIME 800
#define BLINK_TIME 500


static Sprite *createImage(Texture &tex, const char *file, ShaderProgram &program)
{
	tex.loadFromFile(file, TEXTURE_PIXEL_FORMAT_RGBA);
	tex.setMinFilter(GL_NEAREST);
	tex.setMagFilter(GL_NEAREST);
	return Sprite::createSprite(glm::vec2(tex.width(), tex.height()), glm::vec2(1.f, 1.f), &tex, &program);
}

static CreditsLayout layout()
{
	CreditsLayout l;
	l.width = VIRTUAL_WIDTH * TEXT_SCALE;
	l.bottom = VIRTUAL_HEIGHT * TEXT_SCALE + 16.f;
	l.fadeFrom = 64.f;
	l.fadeTo = 16.f;
	l.lastY = 150.f;
	l.speed = 0.02f;
	return l;
}


CreditsScene::CreditsScene()
{
	background = prompt = NULL;
	time = 0;
	bLeaving = false;
}

CreditsScene::~CreditsScene()
{
	delete background;
	delete prompt;
}

void CreditsScene::init()
{
	ShaderProgram &program = Game::instance().getTexProgram();
	background = createImage(backgroundTex, "images/MenuBG.png", program);
	prompt = createImage(promptTex, "images/PressSpace.png", program);
	prompt->setPosition(glm::vec2((VIRTUAL_WIDTH - promptTex.width()) / 2, VIRTUAL_HEIGHT - 14));
	credits.init();
}

void CreditsScene::onEnter()
{
	time = 0;
	bLeaving = false;
	Audio::instance().setMusicDucked(false);
	Audio::instance().setMusicHurry(false);
	Audio::instance().playMusic(MUSIC_CREDITS, true);
}

void CreditsScene::update(int deltaTime)
{
	time += deltaTime;
}

void CreditsScene::render(int windowWidth, int windowHeight)
{
	glm::mat4 projection = setLetterboxViewport(windowWidth, windowHeight, VIRTUAL_WIDTH, VIRTUAL_HEIGHT);
	ShaderProgram &program = Game::instance().getTexProgram();
	program.use();
	program.setUniformMatrix4f("projection", projection);
	program.setUniform4f("color", DIM, DIM, DIM, 1.0f);
	background->render();

	CreditsLayout l = layout();
	int elapsed = time - START_DELAY;
	bool finished = credits.isFinished(elapsed, l);
	if(finished && ((elapsed / BLINK_TIME) % 2 == 0))
	{
		program.use();
		program.setUniformMatrix4f("projection", projection);
		program.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
		prompt->render();
	}
	glm::mat4 textProjection = glm::ortho(0.f, float(VIRTUAL_WIDTH * TEXT_SCALE), float(VIRTUAL_HEIGHT * TEXT_SCALE), 0.f);
	credits.render(textProjection, elapsed, l);
}

void CreditsScene::leave()
{
	if(bLeaving || time < SKIP_TIME)
		return;
	bLeaving = true;
	Audio::instance().playSound(SFX_SELECT);
	Game::instance().changeScene(SCENE_MENU);
}

void CreditsScene::keyPressed(int key)
{
	if(key == GLFW_KEY_SPACE || key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER || key == GLFW_KEY_ESCAPE)
		leave();
}

void CreditsScene::mousePress(int button)
{
	if(button == GLFW_MOUSE_BUTTON_LEFT)
		leave();
}
