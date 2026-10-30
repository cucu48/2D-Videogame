#include <algorithm>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene.h"


// Largest size that keeps the aspect ratio of the virtual screen, centered in the window
static void letterboxRect(int windowWidth, int windowHeight, int virtualWidth, int virtualHeight, int &x, int &y, int &w, int &h)
{
	float scale = std::min(windowWidth / float(virtualWidth), windowHeight / float(virtualHeight));
	w = int(virtualWidth * scale);
	h = int(virtualHeight * scale);
	x = (windowWidth - w) / 2;
	y = (windowHeight - h) / 2;
}

glm::mat4 Scene::setLetterboxViewport(int windowWidth, int windowHeight, int virtualWidth, int virtualHeight)
{
	int x, y, w, h;
	letterboxRect(windowWidth, windowHeight, virtualWidth, virtualHeight, x, y, w, h);
	glViewport(x, y, w, h);

	return glm::ortho(0.f, float(virtualWidth), float(virtualHeight), 0.f);
}

bool Scene::windowToVirtual(int px, int py, int windowWidth, int windowHeight, int virtualWidth, int virtualHeight, glm::vec2 &result)
{
	int x, y, w, h;
	letterboxRect(windowWidth, windowHeight, virtualWidth, virtualHeight, x, y, w, h);
	if(w <= 0 || h <= 0 || px < x || px >= x + w || py < y || py >= y + h)
		return false;
	result = glm::vec2((px - x) * virtualWidth / float(w), (py - y) * virtualHeight / float(h));
	return true;
}
