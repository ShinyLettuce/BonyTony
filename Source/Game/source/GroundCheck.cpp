#include "GroundCheck.h"

namespace GameStateUpdate
{
	void PlayerGroundCheck(
		const std::vector<CrateUpdater::Crate>& aCrates,
		Player& aPlayer, PlayerUpdateResult& aPlayerUpdateResult,
		const std::vector<SceneLoader::TileConfig>& aTileConfigs,
		AnimatedPropUpdater& aPropUpdater
	)
	{
		Tga::Vector2f downUnitVector{ 0.f, -1.f };
		float groundRayMagnitude = 1.f;

		if (aPlayerUpdateResult.velocity.y > 0.f)
		{
			aPlayer.SetGrounded(false); 
			return;
		}

		Physics::Ray playerRayDownLeft = Physics::Ray{
				.origin = {aPlayer.GetPosition().x - 25.f, aPlayer.GetPosition().y - 1.f},
				.direction = downUnitVector,
				.magnitude = groundRayMagnitude
		};

		Physics::Ray playerRayDownRight = Physics::Ray{
				.origin = {aPlayer.GetPosition().x + 25.f, aPlayer.GetPosition().y - 1.f},
				.direction = downUnitVector,
				.magnitude = groundRayMagnitude
		};

		Physics::CollisionResult playerTileRayDownLeft = Physics::RaycastAABBCollisionOverContainer<SceneLoader::TileConfig>(
			playerRayDownLeft,
			aTileConfigs,
			[](const SceneLoader::TileConfig& tile) {
				return Physics::AABB{
					.position = tile.position,
					.size = tile.size
				};
			}
		);

		Physics::CollisionResult playerTileRayDownRight = Physics::RaycastAABBCollisionOverContainer<SceneLoader::TileConfig>(
			playerRayDownRight,
			aTileConfigs,
			[](const SceneLoader::TileConfig& tile) {
				return Physics::AABB{
					.position = tile.position,
					.size = tile.size
				};
			}
		);

		Physics::CollisionResult playerCrateRayDownLeft = Physics::RaycastAABBCollisionOverContainer<CrateUpdater::Crate>(
			playerRayDownLeft,
			aCrates,
			[](const CrateUpdater::Crate& crate) {
				return Physics::AABB{
					.position = crate.position,
					.size = crate.dead? Tga::Vector2f(): crate.size
				};
			}
		);

		Physics::CollisionResult playerCrateRayDownRight = Physics::RaycastAABBCollisionOverContainer<CrateUpdater::Crate>(
			playerRayDownRight,
			aCrates,
			[](const CrateUpdater::Crate& crate) {
				return Physics::AABB{
					.position = crate.position,
					.size = crate.dead ? Tga::Vector2f() : crate.size
				};
			}
		);

		Physics::CollisionResult playerPropRayDownLeft = Physics::RaycastAABBCollisionOverContainer<AnimatedPropUpdater::AnimatedProp>(
			playerRayDownLeft,
			aPropUpdater.GetInteractableProps(),
			[](const AnimatedPropUpdater::AnimatedProp& prop) {

				Tga::Vector3f position = prop.animatedModelInstance->GetTransform().GetPosition();

				return Physics::AABB{
					.position = Tga::Vector2f{position.x, position.y},
					.size = prop.size
				};
			}
		);

		Physics::CollisionResult playerPropRayDownRight = Physics::RaycastAABBCollisionOverContainer<AnimatedPropUpdater::AnimatedProp>(
			playerRayDownRight,
			aPropUpdater.GetInteractableProps(),
			[](const AnimatedPropUpdater::AnimatedProp& prop) {

				Tga::Vector3f position = prop.animatedModelInstance->GetTransform().GetPosition();

				return Physics::AABB{
					.position = Tga::Vector2f{position.x, position.y},
					.size = prop.size
				};
			}
		);

		Physics::CollisionResult* playerLeftCollisions[]{ &playerTileRayDownLeft, &playerCrateRayDownLeft, &playerPropRayDownLeft };
		Physics::CollisionResult* playerLeftCollisionResult{ &playerTileRayDownLeft };

		for (Physics::CollisionResult* result : playerLeftCollisions)
		{
			if (result->pointOfCollisionAlongVelocity < playerLeftCollisionResult->pointOfCollisionAlongVelocity)
			{
				playerLeftCollisionResult = result;
			}
		}

		Physics::CollisionResult* playerRightCollisions[]{ &playerTileRayDownRight, &playerCrateRayDownRight, &playerPropRayDownRight };
		Physics::CollisionResult* playerRightCollisionResult{ &playerTileRayDownRight };

		for (Physics::CollisionResult* result : playerRightCollisions)
		{
			if (result->pointOfCollisionAlongVelocity < playerRightCollisionResult->pointOfCollisionAlongVelocity)
			{
				playerRightCollisionResult = result;
			}
		}

		aPlayer.SetGrounded(false);

		if (playerLeftCollisionResult->didCollide)
		{
			aPlayer.SetGrounded(true);
			aPlayerUpdateResult.position += downUnitVector * groundRayMagnitude * playerLeftCollisionResult->pointOfCollisionAlongVelocity * 0.99f;
		}
		else if (playerRightCollisionResult->didCollide)
		{
			aPlayer.SetGrounded(true);
			aPlayerUpdateResult.position += downUnitVector * groundRayMagnitude * playerRightCollisionResult->pointOfCollisionAlongVelocity * 0.99f;
		}
	}
}