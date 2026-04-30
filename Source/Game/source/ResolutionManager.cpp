#include "ResolutionManager.h"

#include <tge/engine.h>

Tga::Vector2f ResolutionManager::myReferenceResolution = { 1920, 1080 };
Tga::Vector2f ResolutionManager::myCurrentResolution = { 1920, 1080 };
float ResolutionManager::myUIScale = 1.0f;

void ResolutionManager::Init()
{
	Update();
}

void ResolutionManager::Update()
{
	Tga::Engine& engine = *Tga::Engine::GetInstance();
	Tga::Vector2ui renderSize = engine.GetRenderSize();
	myCurrentResolution = { static_cast<float>(renderSize.x), static_cast<float>(renderSize.y) };

	myUIScale = myCurrentResolution.y / myReferenceResolution.y;
}

Tga::Vector2f ResolutionManager::GetScreenPosition(float normalizedX, float normalizedY)
{
	return Tga::Vector2f
	{
		normalizedX * myCurrentResolution.x,
		normalizedY * myCurrentResolution.y
	};
}

Tga::Vector2f ResolutionManager::ScaleSize(float width, float height)
{
	return Tga::Vector2f
	{
		width * myUIScale,
		height * myUIScale
	};
}

float ResolutionManager::ScaleValue(float value)
{
	return value * myUIScale;
}