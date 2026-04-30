#pragma once
#include "State.h"
#include "FullscreenImage.h"
#include "Timer.h"

struct SplashScreenStateHandles
{
	StateHandle mainMenuState;
};

class SplashScreenState : public State
{
public:
	void Init(SplashScreenStateHandles aStateHandles, Timer* aTimer);
	StateUpdateResult Update() override;
	void Render() override;

	void OnGainFocus() override;

private:
	static const int myImageCount = 3;
	int myCurrentImage = 0;

	FullscreenImage myTgaLogo;
	FullscreenImage myAPALogo;
	FullscreenImage myStudioLogo;
	FullscreenImage myImageQueue[myImageCount];

	SplashScreenStateHandles myStateHandles;

	float myLogoTimer = 0.f;

	bool startBufferBool = true;

	bool hasShownTGALogo = false;
	bool hasStartedShowingAPALogo = false;
	bool hasShownAPALogo = false;
	bool hasStartedShowingStudioLogo = false;
	bool hasShownStudioLogo = false;

	float myLogoTimeShowing = 1.f;

	Timer* myTimer;
};