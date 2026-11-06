#ifndef _CREDITS_INCLUDE
#define _CREDITS_INCLUDE


#include <glm/glm.hpp>
#include "Text.h"



struct CreditsLayout
{
	float width;
	float bottom;
	float fadeFrom;
	float fadeTo;
	float lastY;
	float speed;
};


class Credits
{

public:
	Credits();

	bool init();
	void render(const glm::mat4 &projection, int elapsed, const CreditsLayout &layout);
	bool isFinished(int elapsed, const CreditsLayout &layout) const;

private:
	float totalScroll(const CreditsLayout &layout) const;

private:
	Text text;
	bool bText;

};


#endif
