#ifndef _ANIMATION_KEYFRAMES
#define _ANIMATION_KEYFRAMES


#include <vector>


using namespace std;



struct AnimKeyframes
{
	float millisecsPerKeyframe;
	vector<glm::vec2> keyframeDispl;
};


#endif


