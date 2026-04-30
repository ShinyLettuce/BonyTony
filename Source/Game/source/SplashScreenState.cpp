#include "SplashScreenState.h"
#include "Go.h"
#include <tge/engine.h>

void SplashScreenState::Init(SplashScreenStateHandles aStateHandles, Timer* aTimer)
{
	myTimer = aTimer;
	myStateHandles = aStateHandles;

	Tga::Engine::GetInstance()->SetClearColor(Tga::Color{0.f, 0.f, 0.f, 1.f});
	myTgaLogo.Init(FullscreenImageState::Transparent, "textures/UI/Backgrounds/T_TGALogo_C.png");
	myAPALogo.Init(FullscreenImageState::Transparent, "textures/UI/Backgrounds/T_APALogo_C.png");
	myStudioLogo.Init(FullscreenImageState::Transparent, "textures/UI/Backgrounds/T_G4Logo_C.png");
	myTgaLogo.StartFadeOut(1.f);
}

//void SplashScreenState::OnPush()
//{
//	Tga::Engine::GetInstance()->SetClearColor(Tga::Color{0.f, 0.f, 0.f, 1.f});
//	myTgaLogo.Init(FullscreenImageState::Transparent, "textures/UI/Backgrounds/T_TGALogo_C.png");
//	myAPALogo.Init(FullscreenImageState::Transparent, "textures/UI/Backgrounds/T_APALogo_C.png");
//	myTgaLogo.StartFadeOut(3.f);
//}

StateUpdateResult SplashScreenState::Update()
{
	float deltaTime = myTimer->GetDeltaTime();


	if (myTgaLogo.GetState() != FullscreenImageAnimationState::Stopped && !startBufferBool)
	{
		myTgaLogo.Update(deltaTime);
	}

	if (startBufferBool)
	{
		startBufferBool = false;
	}

	if (myTgaLogo.GetAlpha() >= 1.f)
	{
		myLogoTimer += deltaTime;
		if (myLogoTimer >= myTgaLogoTimeShowing)
		{
			myTgaLogo.StartFadeIn(1.f);
			myLogoTimer = 0.f;
			hasShownTGALogo = true;
		}
	}

	if (hasShownTGALogo && myTgaLogo.GetAlpha() < FLT_EPSILON && !hasStartedShowingAPALogo)
	{
		hasStartedShowingAPALogo = true;
		myAPALogo.StartFadeOut(1.f);
	}

	if (myAPALogo.GetState() != FullscreenImageAnimationState::Stopped)
	{
		myAPALogo.Update(deltaTime);
	}

	if (myAPALogo.GetAlpha() >= 1.f)
	{
		myLogoTimer += deltaTime;
		if (myLogoTimer >= myAPALogoTimeShowing)
		{
			myAPALogo.StartFadeIn(1.f);
			myLogoTimer = 0.f;
			hasShownAPALogo = true;
		}
	}

	if (hasShownAPALogo && myAPALogo.GetAlpha() < FLT_EPSILON && !hasStartedShowingStudioLogo)
	{
		hasStartedShowingStudioLogo = true;
		myStudioLogo.StartFadeOut(1.f);
	}

	if (myStudioLogo.GetState() != FullscreenImageAnimationState::Stopped)
	{
		myStudioLogo.Update(deltaTime);
	}

	if (myStudioLogo.GetAlpha() >= 1.f)
	{
		myLogoTimer += deltaTime;
		if (myLogoTimer >= myStudioLogoTimeShowing)
		{
			myStudioLogo.StartFadeIn(1.f);
			myLogoTimer = 0.f;
			hasShownStudioLogo = true;
		}
	}

	if (hasShownTGALogo && hasShownAPALogo  && hasShownStudioLogo && myStudioLogo.GetAlpha() < FLT_EPSILON)
	{
		return StateUpdateResult::CreateClearAndPush(myStateHandles.mainMenuState);
	}

	return StateUpdateResult::CreateContinue();
}

void SplashScreenState::Render()
{
	if (myTgaLogo.GetAlpha() > FLT_EPSILON)
	{
		myTgaLogo.Render();
	}
	if (myAPALogo.GetAlpha() > FLT_EPSILON)
	{
		myAPALogo.Render();
	}
	if (myStudioLogo.GetAlpha() > FLT_EPSILON)
	{
		myStudioLogo.Render();
	}
}

void SplashScreenState::OnGainFocus()
{
	SetMouseCaptureEnabled(false);
}
