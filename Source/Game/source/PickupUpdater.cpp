#include "PickupUpdater.h"

#include <tge/engine.h>
#include <tge/graphics/GraphicsEngine.h>
#include <tge/drawers/ModelDrawer.h>

#include "PopUpState.h"

void PickupUpdater::Init(std::vector<SceneLoader::PickupConfig> aPickupConfigs)
{
	myPickups.clear();

	for (auto& pickup : aPickupConfigs)
	{
		myPickups.emplace_back(Pickup{
				.modelInstance = pickup.modelInstance,
				.position = pickup.position,
				.size = pickup.size,
				.type = pickup.type,
				.isActive = true
			});
	}
}

SceneLoader::PickupType PickupUpdater::Update(Player& aPlayer)
{
	SceneLoader::PickupType pickupType = SceneLoader::PickupType::None;
	
	Physics::CollisionResult collisionResult = Physics::AABBCollisionOverContainer<Pickup>(
		Physics::AABB
		{
			.position = aPlayer.GetPosition(),
			.velocity = aPlayer.GetVelocity(),
			.size = Tga::Vector2f{ 100.0f, 100.0f }
		},
		myPickups,
		[](const Pickup& aPickup)
		{
			return Physics::AABB
			{
				.position = aPickup.position,
				.velocity = Tga::Vector2f(),
				.size = aPickup.isActive ? aPickup.size : Tga::Vector2f(),
			};
		});

	if (collisionResult.didCollide)
	{
		Pickup& pickup = myPickups.at(collisionResult.indexToEntityCollidedWith);
		switch (pickup.type)
		{
			case SceneLoader::PickupType::Shotgun:
			{
				std::cout << "[PickupUpdater.cpp] Added shotgun" << '\n';
				aPlayer.EnableShotgun();
				pickupType = SceneLoader::PickupType::Shotgun;
				break;
			}
			case SceneLoader::PickupType::Revolver:
			{
				std::cout << "[PickupUpdater.cpp] Added revolver" << '\n';
				aPlayer.EnableRevolver();
				pickupType = SceneLoader::PickupType::Revolver;
				break;
			}
			case SceneLoader::PickupType::PowerShot:
			{
				std::cout << "[PickupUpdater.cpp] Added powershot" << '\n';
				aPlayer.EnablePowershot();
				pickupType = SceneLoader::PickupType::PowerShot;
				break;
			}
			default:
			{
				std::cout << "[PickupUpdater.cpp] Warning: Cant add unknown pickup type" << '\n';
				break;
			}
		}
		pickup.isActive = false;
	}
	
	return pickupType;
}

void PickupUpdater::Render()
{
	for (const auto& object : myPickups)
	{
		if (object.isActive)
		{
			const Tga::Engine& engine = *Tga::Engine::GetInstance();
			Tga::GraphicsEngine& graphicsEngine = engine.GetGraphicsEngine();
			Tga::ModelDrawer& modelDrawer = graphicsEngine.GetModelDrawer();

			modelDrawer.Draw(object.modelInstance);
		}
	}
}
