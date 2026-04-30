#pragma once
#include <tge/sprite/sprite.h>
#include <string_view>

enum class FullscreenImageAnimationState
{
	FadeIn,
	FadeOut,
	Stopped
};

enum class FullscreenImageState
{
	Opaque,
	Transparent
};

class FullscreenImage
{
public:
	void Init(FullscreenImageState aStartState, std::string_view aImage);

	void StartFadeIn(float aAnimationTime);
	void StartFadeOut(float aAnimationTime);

	float GetAlpha() {return myTimer / myAnimationTime; }
	FullscreenImageAnimationState GetState() { return myState; }

	void Update(float aDeltaTime);
	void Render();
private:
	float myAnimationTime;
	float myTimer;

	Tga::SpriteSharedData myImage;

	FullscreenImageAnimationState myState;
};