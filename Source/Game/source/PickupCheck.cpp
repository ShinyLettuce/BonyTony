#include "PickupCheck.h"

namespace GameStateUpdate
{
	void PlayerPickupCheck(Player& aPlayer, std::vector<SceneLoader::PickupConfig>& aPickupConfigs)
	{
		Physics::CollisionResult collisionResult = Physics::AABBCollisionOverContainer<SceneLoader::PickupConfig>(
			Physics::AABB
			{
				.position = aPlayer.GetPosition(),
				.velocity = aPlayer.GetVelocity(),
				.size = Tga::Vector2f{ 100.0f, 100.0f }
			},
			aPickupConfigs,
			[](const SceneLoader::PickupConfig& aPickupConfig)
			{
				(aPickupConfig);
				return Physics::AABB
				{
					.position = aPickupConfig.position,
					.velocity = Tga::Vector2f{ 0.0f, 0.0f },
					.size = aPickupConfig.size,
				};
			});

		if (collisionResult.didCollide)
		{
			SceneLoader::PickupConfig& pickupConfig = aPickupConfigs.at(collisionResult.indexToEntityCollidedWith);
			switch (pickupConfig.type)
			{
				case SceneLoader::PickupType::Shotgun:
				{
					std::cout << "Shotgun" << '\n';
					break;
				}
				case SceneLoader::PickupType::Revolver:
				{
					std::cout << "Revolver" << '\n';
					break;
				}
				case SceneLoader::PickupType::PowerShot:
				{
					std::cout << "Powershot" << '\n';
					break;
				}
				default:
				{
					std::cout << "Unknown" << '\n';
					break;
				}
			}
		}
	}
}