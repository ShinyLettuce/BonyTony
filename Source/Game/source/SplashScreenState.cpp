#include "SplashScreenState.h"
#include "Go.h"
#include <tge/engine.h>

void SplashScreenState::Init(SplashScreenStateHandles aStateHandles, Timer* aTimer) // TODO: Change this to happen on push
{
	myTimer = aTimer;
	myStateHandles = aStateHandles;

	Tga::Engine::GetInstance()->SetClearColor(Tga::Color{ 0.f, 0.f, 0.f, 1.f });
	myTgaLogo.Init(FullscreenImageState::Transparent, "textures/UI/Backgrounds/T_TGALogo_C.png");
	myAPALogo.Init(FullscreenImageState::Transparent, "textures/UI/Backgrounds/T_APALogo_C.png");
	myStudioLogo.Init(FullscreenImageState::Transparent, "textures/UI/Backgrounds/T_G4Logo_C.png");

	myImageQueue[0] = myTgaLogo;
	myImageQueue[1] = myAPALogo;
	myImageQueue[2] = myStudioLogo;

	myImageQueue[0].StartFadeOut(1.f);
}

StateUpdateResult SplashScreenState::Update()
{
	float deltaTime = myTimer->GetDeltaTime();

	if (startBufferBool)
	{
		startBufferBool = false;
		return StateUpdateResult::CreateContinue();;
	}

	if (myImageQueue[myCurrentImage].GetState() != FullscreenImageAnimationState::Stopped)
	{
		myImageQueue[myCurrentImage].Update(deltaTime);
	}

	if (myImageQueue[myCurrentImage].GetAlpha() >= 1.f)
	{
		myLogoTimer += deltaTime;
		if (myLogoTimer >= myLogoTimeShowing)
		{
			myImageQueue[myCurrentImage].StartFadeIn(1.f);
		}
	}
	if (myImageQueue[myCurrentImage].GetAlpha() < FLT_EPSILON && myLogoTimer > 0.f)
	{
		myCurrentImage++;

		if (myCurrentImage == myImageCount)
		{
			return StateUpdateResult::CreateClearAndPush(myStateHandles.mainMenuState);
		}

		myLogoTimer = 0.f;
		myImageQueue[myCurrentImage].StartFadeOut(1.f);
	}
	return StateUpdateResult::CreateContinue();
}

void SplashScreenState::Render()
{
	myImageQueue[myCurrentImage].Render();
}

void SplashScreenState::OnGainFocus()
{
	SetMouseCaptureEnabled(false);
}
