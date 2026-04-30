#include "FullscreenImage.h"
#include <tge/engine.h>
#include <tge/graphics/GraphicsEngine.h>
#include <tge/drawers/SpriteDrawer.h>
#include <tge/texture/TextureManager.h>



void FullscreenImage::Init(FullscreenImageState aStartState, std::string_view aImage)
{
	Tga::Engine* engine = Tga::Engine::GetInstance();
	myImage.myTexture = engine->GetTextureManager().GetTexture(aImage.data());
	myState = FullscreenImageAnimationState::Stopped;

	switch (aStartState)
	{
		case FullscreenImageState::Opaque:
		{
			myTimer = myAnimationTime;
			break;
		}
		case FullscreenImageState::Transparent:
		{
			myTimer = 0.f;
			break;
		}
	}
}

void FullscreenImage::StartFadeIn(float aAnimationTime)
{
	myAnimationTime = aAnimationTime;
	myTimer = myAnimationTime;
	myState = FullscreenImageAnimationState::FadeIn;
}

void FullscreenImage::StartFadeOut(float aAnimationTime)
{
	myTimer = 0.0f;
	myAnimationTime = aAnimationTime;
	myState = FullscreenImageAnimationState::FadeOut;
}

void FullscreenImage::Update(float aDeltaTime)
{
	switch (myState)
	{
		case FullscreenImageAnimationState::FadeIn:
		{
			myTimer -= aDeltaTime;
			if (myTimer <= 0.0f)
			{
				myState = FullscreenImageAnimationState::Stopped;
			}
			break;
		}
		case FullscreenImageAnimationState::FadeOut:
		{
			myTimer += aDeltaTime;
			if (myTimer >= myAnimationTime)
			{
				myState = FullscreenImageAnimationState::Stopped;
			}
			break;
		}
		case FullscreenImageAnimationState::Stopped:
		{
			//nothing lol
			break;
		}
	}

	
}

void FullscreenImage::Render()
{
	Tga::Engine& engine = *Tga::Engine::GetInstance();

	float alpha = myTimer / myAnimationTime;

	Tga::Sprite2DInstanceData myDrawCall = {
		.myPosition = static_cast<Tga::Vector2f>(engine.GetRenderSize()) * 0.5f,
		.myPivot = 0.5f,
		.mySize = static_cast<Tga::Vector2f>(engine.GetRenderSize()),
		.myColor = Tga::Color(1.f, 1.f, 1.f, alpha)
	};

	engine.GetGraphicsEngine().GetSpriteDrawer().Draw(myImage, myDrawCall);
}
