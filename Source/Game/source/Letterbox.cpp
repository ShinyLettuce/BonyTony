#include "Letterbox.h"

#include <tge/engine.h>
#include <tge/graphics/GraphicsEngine.h>
#include <tge/drawers/SpriteDrawer.h>
#include <tge/texture/TextureManager.h>

#include "MathUtils.h"

Letterbox::Letterbox()
{
	const Tga::Engine& engine = *Tga::Engine::GetInstance();
	Tga::TextureManager& textureManager = engine.GetTextureManager();

	myLetterboxSharedData.myTexture = textureManager.GetTexture("Sprites/Pixel.png");
}

void Letterbox::Activate()
{
	myIsActive = true;
	myTimeActive = 0.0f;
}

void Letterbox::Update(float aDeltaTime)
{
	myTimeActive += aDeltaTime;
}

void Letterbox::Render()
{
	constexpr float speed = 43.0f;
	constexpr float aspectPercent = 0.2f;

	const Tga::Engine& engine = *Tga::Engine::GetInstance();
	Tga::GraphicsEngine& graphicsEngine = engine.GetGraphicsEngine();
	Tga::SpriteDrawer& spriteDrawer = graphicsEngine.GetSpriteDrawer();

	Tga::Vector2ui renderSize = engine.GetRenderSize();
	Tga::Vector2f resolution = Tga::Vector2f{ static_cast<float>(renderSize.x), static_cast<float>(renderSize.y) };

	const float barWidth = resolution.x;
	const float barHeight = resolution.y * aspectPercent;

	const float height = MathUtils::Min(myTimeActive * speed, barHeight);

	Tga::Sprite2DInstanceData topInstanceData;
	topInstanceData.myPivot = Tga::Vector2f{ 0.0f, 0.0f };
	topInstanceData.myPosition = Tga::Vector2f{ 0.0f, resolution.y + barHeight - height  };
	topInstanceData.mySize = Tga::Vector2f{ barWidth, barHeight };
	topInstanceData.myColor = Tga::Color{ 0.0f, 0.0f, 0.0f, 1.0f };

	spriteDrawer.Draw(myLetterboxSharedData, topInstanceData);

	Tga::Sprite2DInstanceData bottomInstanceData;
	bottomInstanceData.myPivot = Tga::Vector2f{ 0.0f, 0.0f };
	bottomInstanceData.myPosition = Tga::Vector2f{ 0.0f, height };
	bottomInstanceData.mySize = Tga::Vector2f{ barWidth, barHeight };
	bottomInstanceData.myColor = Tga::Color{ 0.0f, 0.0f, 0.0f, 1.0f };

	spriteDrawer.Draw(myLetterboxSharedData, bottomInstanceData);
}
