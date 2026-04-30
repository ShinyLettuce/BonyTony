#pragma once

#include "tge/sprite/sprite.h"

class Letterbox
{
public:
	Letterbox();

	void Activate();

	void Update(float aDeltaTime);
	void Render();
private:
	Tga::SpriteSharedData myLetterboxSharedData;

	bool myIsActive = false;
	float myTimeActive = 0.0f;
};