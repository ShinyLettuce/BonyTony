#include "HUD.h"

#include "Camera.h"
#include "tge/Engine.h"
#include "tge/drawers/SpriteDrawer.h"
#include "tge/graphics/GraphicsEngine.h"
#include "tge/texture/TextureManager.h"
#include "ResolutionManager.h"
#include "tge/drawers/LineDrawer.h"
#include <tge/graphics/DX11.h>

#include "Options.h"
#include "imgui/imgui.h"

void HUD::Init(const int aShotgunMaxClip, const int aRevolverMaxClip, float const aAimMagnitude)
{
	myShellInstances.clear();
	myBulletInstances.clear();

	myAimMagnitude = aAimMagnitude;

	auto& textureManager = Tga::Engine::GetInstance()->GetTextureManager();

	myAimlineData.myTexture = textureManager.GetTexture(UI.aimLineTexture);          
	myHitPointData.myTexture = textureManager.GetTexture(UI.crosshairTexture);       

	myShellData.myTexture = textureManager.GetTexture(UI.shellTexture);              
	mySpentShellData.myTexture = textureManager.GetTexture(UI.spentShellTexture);    
	myBulletData.myTexture = textureManager.GetTexture(UI.bulletTexture);            
	mySpentBulletData.myTexture = textureManager.GetTexture(UI.spentBulletTexture);  

	myShellInstances.resize(aShotgunMaxClip);
	myBulletInstances.resize(aRevolverMaxClip);

	PositionElements(aShotgunMaxClip, aRevolverMaxClip);
}

void HUD::UpdateAimLine(const AimLineContext& aContext)
{
	const Physics::CollisionResult aimToTiles =
		Physics::RaycastAABBCollisionOverContainer<SceneLoader::TileConfig>(
			Physics::Ray
			{
				.origin = aContext.aimOrigin,
				.direction = aContext.aimDirection,
				.magnitude = myAimMagnitude,
			},
			aContext.tiles,
			[](const SceneLoader::TileConfig& aTile)
			{
				return Physics::AABB
				{
					.position = aTile.position,
					.velocity = Tga::Vector2f{ 0.0f, 0.0f },
					.size = aTile.size,
				};
			}
		);

	const Physics::CollisionResult aimToCrates =
		Physics::RaycastAABBCollisionOverContainer<CrateUpdater::Crate>(
			Physics::Ray
			{
				.origin = aContext.aimOrigin,
				.direction = aContext.aimDirection,
				.magnitude = myAimMagnitude,
			},
			aContext.crates,
			[](const CrateUpdater::Crate& aCrate)
			{
				if (!aCrate.dead)
				{
					return Physics::AABB
					{
						.position = aCrate.position,
						.velocity = Tga::Vector2f{ 0.0f, 0.0f },
						.size = aCrate.size,
					};
				}
				return Physics::AABB{};
			}
		);

	const Physics::CollisionResult aimToEnemies =
		Physics::RaycastAABBCollisionOverContainer<Enemy>(
			Physics::Ray
			{
				.origin = aContext.aimOrigin,
				.direction = aContext.aimDirection,
				.magnitude = myAimMagnitude,
			},
			aContext.enemies,
			[](const Enemy& aEnemy)
			{
				if (aEnemy.GetIsAlive())
				{
					return Physics::AABB
					{
						.position = aEnemy.GetPosition(),
						.velocity = Tga::Vector2f{ 0.0f, 0.0f },
						.size = aEnemy.GetSize(),
					};
				}
				return Physics::AABB{};
			}
		);

	float distanceToClosestCollision =
		std::min
		(
			{
				aimToEnemies.pointOfCollisionAlongVelocity,
				aimToCrates.pointOfCollisionAlongVelocity,
				aimToTiles.pointOfCollisionAlongVelocity
			}
		)
	;

	
	Aimline* aimline;
	
	if (aContext.type == AimLineType::Second)
	{
		aimline = &mySecondAimline;
	}
	else
	{
		aimline = &myFirstAimline;
	}
	
	aimline->shouldRender = true;
	
	const Tga::Vector2f up{ 0.f, 1.f };
	const float angle = std::atan2f(up.Cross(aContext.aimDirection), up.Dot(aContext.aimDirection));
	const float uiScale = ResolutionManager::GetUIScale(); 

	Tga::Vector2f baseAimlineSize{ myAimlineData.myTexture->CalculateTextureSize() };
	Tga::Vector2f scaledAimlineSize = baseAimlineSize * uiScale;

	for (int i = 0; i < static_cast<int>(Aimline::SPRITE_AMOUNT); i++)
	{
		aimline->aimlineInstances.at(i).myRotation = angle;
		aimline->aimlineInstances.at(i).myPosition = aContext.aimOrigin + aContext.aimDirection * (scaledAimlineSize.y * static_cast<float>(i + 1) + (UI.aimLineGapSize * uiScale));
		aimline->aimlineInstances.at(i).mySize = scaledAimlineSize;
	}

	aimline->origin = aContext.aimOrigin;
	aimline->end = aContext.aimOrigin + aContext.aimDirection * myAimMagnitude * distanceToClosestCollision;

	const Tga::Vector2f hitTexSize{ myHitPointData.myTexture->CalculateTextureSize() }; 

	if (distanceToClosestCollision == aimToEnemies.pointOfCollisionAlongVelocity && aContext.enemies.size() != 0)
	{
		Tga::Vector2f pos = aContext.enemies.at(aimToEnemies.indexToEntityCollidedWith).GetPosition();
		Tga::Vector2f size = aContext.enemies.at(aimToEnemies.indexToEntityCollidedWith).GetSize();
		aimline->hitPointInstance.myPosition = { pos.x, pos.y + (size.y * 0.5f) };

		aimline->hitPointInstance.myColor = { 249.f / 255.f, 147.f / 255.f, 157.f / 255.f, 1.f };
		aimline->hitPointInstance.mySize = hitTexSize * uiScale * UI.hitPointScaleHighlight; 
	}
	else if (distanceToClosestCollision == aimToCrates.pointOfCollisionAlongVelocity && aContext.crates.size() != 0)
	{
		Tga::Vector2f pos = aContext.crates.at(aimToCrates.indexToEntityCollidedWith).position;
		Tga::Vector2f size = aContext.crates.at(aimToCrates.indexToEntityCollidedWith).size;
		aimline->hitPointInstance.myPosition = { pos.x, pos.y + (size.y * 0.5f) };

		aimline->hitPointInstance.myColor = { 0.f, 1.f, 0.f, 1.f };  
		aimline->hitPointInstance.mySize = hitTexSize * uiScale * UI.hitPointScaleHighlight;
	}
	else
	{
		aimline->hitPointInstance.myPosition = aimline->end;

		aimline->hitPointInstance.myColor = { 1.f, 1.f, 1.f, 1.f };
		aimline->hitPointInstance.mySize = hitTexSize * uiScale * UI.hitPointScaleNormal; 
	}
}

void HUD::RenderClips(const int aShotgunClip, const bool aRevolverReady, const int aRevolverClip) const
{
	const Tga::Engine& engine = *Tga::Engine::GetInstance();
	Tga::GraphicsEngine& graphicsEngine = engine.GetGraphicsEngine();
	Tga::SpriteDrawer& spriteDrawer = graphicsEngine.GetSpriteDrawer();

	Tga::DX11::SetDepthEnabled(false);

	for (int i = 0; i < myShellInstances.size(); i++)
	{
		if (i < aShotgunClip)
		{
			spriteDrawer.Draw(myShellData, myShellInstances.at(i));
		}
		else
		{
			spriteDrawer.Draw(mySpentShellData, myShellInstances.at(i));
		}
	}

	if (!aRevolverReady)
	{
		Tga::DX11::SetDepthEnabled(true);
		return;
	}

	for (int i = 0; i < myBulletInstances.size(); i++)
	{
		if (i < aRevolverClip)
		{
			spriteDrawer.Draw(myBulletData, myBulletInstances.at(i));
		}
		else
		{
			spriteDrawer.Draw(mySpentBulletData, myBulletInstances.at(i));
		}
	}

	Tga::DX11::SetDepthEnabled(true);
}

void HUD::RenderAimline()
{
	const Tga::Engine& engine = *Tga::Engine::GetInstance();
	Tga::GraphicsEngine& graphicsEngine = engine.GetGraphicsEngine();
	Tga::SpriteDrawer& spriteDrawer = graphicsEngine.GetSpriteDrawer();

	Tga::DX11::SetDepthEnabled(false);

	if (myFirstAimline.shouldRender)
	{
		for (int i = 0; i < static_cast<int>(Aimline::SPRITE_AMOUNT); i++)
		{
			if (Tga::Vector2f::Distance(myFirstAimline.origin, myFirstAimline.aimlineInstances.at(i).myPosition) < (myFirstAimline.end - myFirstAimline.origin).Length())
			{
				spriteDrawer.Draw(myAimlineData, myFirstAimline.aimlineInstances.at(i));
			}
			else
			{
				break;
			}
		}
	}
	if (mySecondAimline.shouldRender)
	{
		for (int i = 0; i < static_cast<int>(Aimline::SPRITE_AMOUNT); i++)
		{
			if (Tga::Vector2f::Distance(mySecondAimline.origin, mySecondAimline.aimlineInstances.at(i).myPosition) < (mySecondAimline.end - mySecondAimline.origin).Length())
			{
				spriteDrawer.Draw(myAimlineData, mySecondAimline.aimlineInstances.at(i));
			}
			else
			{
				break;
			}
		}
	}
	
	Tga::DX11::SetDepthEnabled(true);
}

void HUD::RenderHitPoint(Camera& aCamera)
{
	const Tga::Engine& engine = *Tga::Engine::GetInstance();
	Tga::GraphicsEngine& graphicsEngine = engine.GetGraphicsEngine();
	Tga::SpriteDrawer& spriteDrawer = graphicsEngine.GetSpriteDrawer();

	Tga::DX11::SetDepthEnabled(false);

	if (myFirstAimline.shouldRender)
	{
		myFirstAimline.hitPointInstance.myPosition = aCamera.GetWorldToScreenPoint(myFirstAimline.hitPointInstance.myPosition);
		spriteDrawer.Draw(myHitPointData, myFirstAimline.hitPointInstance);
	}
	
	if (mySecondAimline.shouldRender)
	{
		mySecondAimline.hitPointInstance.myPosition = aCamera.GetWorldToScreenPoint(mySecondAimline.hitPointInstance.myPosition);
		spriteDrawer.Draw(myHitPointData, mySecondAimline.hitPointInstance);
	}
	
	myFirstAimline.shouldRender = false;
	mySecondAimline.shouldRender = false;
	Tga::DX11::SetDepthEnabled(true);
}

void HUD::PositionElements(const int aShotgunMaxClip, const int aRevolverMaxClip)
{
	const auto& engine = *Tga::Engine::GetInstance();
	const auto renderSize = engine.GetRenderSize();
	const Tga::Vector2f resolution = { static_cast<float>(renderSize.x), static_cast<float>(renderSize.y) };

	Tga::Vector2f baseShellSize{ myShellData.myTexture->CalculateTextureSize() };
	Tga::Vector2f baseBulletSize{ myBulletData.myTexture->CalculateTextureSize() };

	float uiScale = ResolutionManager::GetUIScale();

	Tga::Vector2f scaledShellSize = { baseShellSize.x * uiScale * UI.shellSizeMultiplier, baseShellSize.y * uiScale * UI.shellSizeMultiplier };   
	Tga::Vector2f scaledBulletSize = { baseBulletSize.x * uiScale * UI.bulletSizeMultiplier, baseBulletSize.y * uiScale * UI.bulletSizeMultiplier }; 

	const float shellLeftX = ResolutionManager::ScaleValue(UI.shellLeftMarginRef); 
	const float bulletRightX = resolution.x - ResolutionManager::ScaleValue(UI.bulletRightMarginRef); 
	const float shellBottomY = ResolutionManager::ScaleValue(UI.shellBottomMarginRef); 
	const float bulletBottomY = ResolutionManager::ScaleValue(UI.bulletBottomMarginRef); 

	const float shellStepY = scaledShellSize.y + ResolutionManager::ScaleValue(UI.shellSpacingRef); 
	const float bulletStepY = scaledBulletSize.y + ResolutionManager::ScaleValue(UI.bulletSpacingRef); 

	for (int i = 0; i < aShotgunMaxClip; i++)
	{
		myShellInstances[i].myPivot = { 0.f, 0.f };
		myShellInstances[i].mySize = scaledShellSize;
		myShellInstances[i].myPosition = {
			shellLeftX,
			shellBottomY + shellStepY * static_cast<float>(i)
		};
	}

	for (int i = 0; i < aRevolverMaxClip; i++)
	{
		myBulletInstances[i].myPivot = { 1.f, 0.f };
		myBulletInstances[i].mySize = scaledBulletSize;
		myBulletInstances[i].myPosition = {
			bulletRightX,
			bulletBottomY + bulletStepY * static_cast<float>(i)
		};
	}

	// aimline
	// Tga::Vector2f baseAimlineSize{ myAimlineData.myTexture->CalculateTextureSize() };
	// Tga::Vector2f scaledAimlineSize = baseAimlineSize * uiScale;
	//
	// float aimlineSpriteAmount = myAimMagnitude / (scaledAimlineSize.y + UI.aimLineGapSize); 
	//
	// Tga::Sprite2DInstanceData instance;
	// for (int i = 0; i < static_cast<int>(aimlineSpriteAmount); i++)
	// {
	// 	instance.mySize = scaledAimlineSize;
	// 	instance.myPivot = { 0.5f, 0.f };
	// 	myAimlineInstancesShotgun.push_back(instance);
	// 	myAimlineInstancesRevolver.push_back(instance);
	// }

	myFirstAimline.hitPointInstance.mySize = Tga::Vector2f{ myHitPointData.myTexture->CalculateTextureSize() } *uiScale * UI.hitPointScaleHighlight; 
	myFirstAimline.hitPointInstance.myPivot = { 0.5f, 0.5f };
	
	mySecondAimline.hitPointInstance.mySize = Tga::Vector2f{ myHitPointData.myTexture->CalculateTextureSize() } *uiScale * UI.hitPointScaleHighlight; 
	mySecondAimline.hitPointInstance.myPivot = { 0.5f, 0.5f };
}